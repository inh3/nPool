var DOMParser = require(__dirname + './dom-parser.js').DOMParser;
var XML2JSON = require(__dirname + './xml2json.js');

function XMLParser () {

    this.parseXmlData = function (xmlString) {

        var xmlDoc = new DOMParser().parseFromString(xmlString.xmlData, 'text/xml');
        var json = XML2JSON(xmlDoc);

        console.log('*** XML Parser ***');

        return { jsonObject: json };
    };
}

module.exports = XMLParser;