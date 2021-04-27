/**
  jsI18n library.
  Simple client side internationalization.
  Version: 1.1.1
  Copyright (c) Daniel Abrahamsson 2010
**/

function JsI18n()
{
  this.locale = "";            //Current locale
  this.locales = [];  //Available locales

  /*
    Method for automatically detecting the language, does not work in every browser.
  */
  this.detectLanguage = function(successCB, errorCB) {
    // Phonegap browser detection
    if (navigator.globalization !== null && navigator.globalization !== undefined) {
      navigator.globalization.getPreferredLanguage(
        function (language) { successCB(language); },
        function (error) { errorCB(error); }
      );
    //Normal browser detection
    } else {
      if(window.navigator.language !== null && window.navigator.language !== undefined) {
          successCB(window.navigator.language);
      }
    }
  };


  /*
    Helper for translating a node
    and all its child nodes.
  */
  this.processNode = function(node)
  {
    if(node != undefined)
    {
      if(node.nodeType == 1) //Element node
      {
        var key = node.attributes["data-trans"];
        if(key != null)
          translateTag(node, key.nodeValue);
      }
    
      //Process child nodes
      var children = node.childNodes;
      for(var i = 0; i < children.length; i++)
        this.processNode(children[i]);
    } 

    /*
      Translates tag contents and
      attributes depending on the
      value of key.
    */
    function translateTag(node, key)
    {
      if(key.indexOf("=") == -1) //Simple key
        translateNodeContent(node, key);
      else //Attribute/key pairs
      {
        var parts = key.toLowerCase().split(";");
        
        for(var i = 0; i < parts.length; i++)
        {
          var pair = parts[i].split("=");
          var attr = pair[0].replace(/\s*(\w+)\s*/gi, "$1"); //trim
          var k = pair[1].replace(/\s*(\.+)\s*/gi, "$1");
          
          if(attr == "html")
            translateNodeContent(node, k);
          else
            translateNodeContent(node.attributes[attr], k);
        }
      }
    }
    
    /**
      Replace the content of the given node
      if there is a translation for the given key.
    **/
    function translateNodeContent(node, key)
    {
      var translation = jsI18n.t(key); //Hack, "this" does not work
      if(node != null && translation != undefined) 
      {
        if(node.nodeType == 1) //Element
        {
          try {  
            node.innerHTML = translation;
          } catch(e) {
            node.text = translation;
          }
        }
        else if(node.nodeType == 2) //Attribute
          node.value = translation;
      }
    }
  };
}

/*
  Adds a locale to the list,
  replacing the translations
  if the locale is already defined.
*/
JsI18n.prototype.addLocale = function(locale, translations)
{
  this.locales[locale.toString()] = translations;
};

/*
  Sets the locale to use when translating.
*/
JsI18n.prototype.setLocale = function(locale)
{
  this.locale = locale;
};

/*
  Fetches the translation associated with the given key.
*/
JsI18n.prototype.t = function(key)
{
  var translations = this.locales[this.locale];
  if(translations != undefined)
  {
    return translations[key.toString()];
  }
  return undefined;
};

/*
  Alias for JsI18n.t
*/
JsI18n.prototype.translate = function(key)
{
  this.t(key);
};

/**
  Replaces the contents of all tags
  that have the data-trans attribute set.
**/
JsI18n.prototype.processPage = function()
{
  this.processNode(document.getElementsByTagName("html")[0]);
};

//Global
jsI18n = new JsI18n();