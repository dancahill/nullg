function replaceSubstring(inputString, fromString, toString) {
   // Goes through the inputString and replaces every occurrence of fromString with toString
   var temp = inputString;
   if (fromString == "") {
      return inputString;
   }
   if (toString.indexOf(fromString) == -1) { // If the string being replaced is not a part of the replacement string (normal situation)
      while (temp.indexOf(fromString) != -1) {
         var toTheLeft = temp.substring(0, temp.indexOf(fromString));
         var toTheRight = temp.substring(temp.indexOf(fromString)+fromString.length, temp.length);
         temp = toTheLeft + toString + toTheRight;
      }
   } else { // String being replaced is part of replacement string (like "+" being replaced with "++") - prevent an infinite loop
      var midStrings = new Array("~", "`", "_", "^", "#");
      var midStringLen = 1;
      var midString = "";
      // Find a string that doesn't exist in the inputString to be used
      // as an "inbetween" string
      while (midString == "") {
         for (var i=0; i < midStrings.length; i++) {
            var tempMidString = "";
            for (var j=0; j < midStringLen; j++) { tempMidString += midStrings[i]; }
            if (fromString.indexOf(tempMidString) == -1) {
               midString = tempMidString;
               i = midStrings.length + 1;
            }
         }
      } // Keep on going until we build an "inbetween" string that doesn't exist
      // Now go through and do two replaces - first, replace the "fromString" with the "inbetween" string
      while (temp.indexOf(fromString) != -1) {
         var toTheLeft = temp.substring(0, temp.indexOf(fromString));
         var toTheRight = temp.substring(temp.indexOf(fromString)+fromString.length, temp.length);
         temp = toTheLeft + midString + toTheRight;
      }
      // Next, replace the "inbetween" string with the "toString"
      while (temp.indexOf(midString) != -1) {
         var toTheLeft = temp.substring(0, temp.indexOf(midString));
         var toTheRight = temp.substring(temp.indexOf(midString)+midString.length, temp.length);
         temp = toTheLeft + toString + toTheRight;
      }
   } // Ends the check to see if the string being replaced is part of the replacement string or not
   return temp; // Send the updated string back to the user
} // Ends the "replaceSubstring" function

function browser_version()
{
	var useragent = navigator.userAgent;
	var bName = (useragent.indexOf('Opera') > -1) ? 'Opera' : navigator.appName;
	var pos = useragent.indexOf('MSIE');
	if (pos>-1) {
		bVer=useragent.substring(pos+5);
		var pos=bVer.indexOf(';');
		var bVer=bVer.substring(0, pos);
	}
	var pos=useragent.indexOf('Opera');
	if (pos>-1) {
		bVer=useragent.substring(pos+6);
		var pos=bVer.indexOf(' ');
		var bVer=bVer.substring(0, pos);
	}
	if (bName=="Netscape") {
		var bVer=useragent.substring(8);
		var pos=bVer.indexOf(' ');
		var bVer=bVer.substring(0, pos);
	}
	if (bName=="Netscape"&&parseInt(navigator.appVersion)>=5) {
		var pos=useragent.lastIndexOf('/');
		var bVer=useragent.substring(pos+1);
	}
	return (new Array(bName, bVer));
}
var user_agent=browser_version();
var BrowserSupportsHTMLEditor;

BrowserSupportsHTMLEditor=(user_agent[0]=="Microsoft Internet Explorer"&&user_agent[1]>=4)?true:false;
BrowserSupportsHTMLEditor=(user_agent[0]=="Netscape"&&user_agent[1]>=20030312)?true:BrowserSupportsHTMLEditor;
EditorMode="PLAIN";

function init()
{
	if (!BrowserSupportsHTMLEditor) return;
	document.wmcompose.ctype.value="plain";
	if (user_agent[0]=="Microsoft Internet Explorer") {
		document.getElementById("wmeditor").contentWindow.document.designMode="on";
	}
}

function toggle_mode()
{
	var editor;

	if (!BrowserSupportsHTMLEditor) return false;
	if (document.wmcompose.ctype.value=="plain") {
		editor=document.getElementById("wmeditor").contentWindow.document;
		EditorMode="PLAIN";
		if (user_agent[0]=="Microsoft Internet Explorer") {
			document.wmcompose.msgbody.value=editor.body.innerText;
		} else {
			document.wmcompose.msgbody.value=editor.body.innerHTML;
		}
		document.wmcompose.msgbody.value=""+replaceSubstring(document.wmcompose.msgbody.value, "<BR>", "\r\n")+"";
		document.wmcompose.msgbody.value=""+replaceSubstring(document.wmcompose.msgbody.value, "&gt;", ">")+"";
		document.wmcompose.msgbody.value=""+replaceSubstring(document.wmcompose.msgbody.value, "&nbsp;", " ")+"";
		editor.body.style.fontFamily="courier";
		editor.body.style.fontSize="9pt";
		if (user_agent[0]=="Netscape") {
			document.getElementById("wmeditor").contentWindow.document.designMode="off";
		}
		document.getElementById("MenuBar").style.display="none";
		document.wmcompose.msgbody.style.display="inline";
		document.wmcompose.msgbody.focus();
	} else {
		EditorMode="HTML";
		document.getElementById("MenuBar").style.display="block";
		if (user_agent[0]=="Netscape") {
			document.getElementById("wmeditor").contentWindow.document.designMode="on";
		}
		editor=document.getElementById("wmeditor").contentWindow.document;
		editor.body.innerText=document.wmcompose.msgbody.value;
		editor.body.innerHTML=replaceSubstring(editor.body.innerText, "\r\n", "<BR>");
		document.wmcompose.msgbody.style.display="none";
		editor.body.style.fontFamily="";
		editor.body.style.fontSize="";
		document.getElementById("wmeditor").contentWindow.focus();
}	}

function copy_submit()
{
	if (!BrowserSupportsHTMLEditor) return false;
	if (EditorMode=="HTML") {
		document.wmcompose.msgbody.value=""+document.getElementById("wmeditor").contentWindow.document.body.innerHTML+"";
	}
}

function wmformat(what)
{
	if (!BrowserSupportsHTMLEditor) return;
	if (EditorMode!="HTML") return;
	document.getElementById("wmeditor").contentWindow.document.execCommand(what, false, null);
	document.getElementById("wmeditor").contentWindow.focus();
}

function html_action(what)
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById("wmeditor").contentWindow.document.execCommand(what);
}

function doImage()
{
	var imgSrc;

	if (!BrowserSupportsHTMLEditor) return;
	imgSrc=prompt('Enter Image URL', '');
	if (imgSrc!=null) {
		document.getElementById("wmeditor").contentWindow.document.execCommand('InsertImage', false, imgSrc);
	}
}

function doLink()
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById("wmeditor").contentWindow.document.execCommand('CreateLink');
}

function SetForeColour()
{
	var fCol;

	if (!BrowserSupportsHTMLEditor) return;
	fCol=prompt('Enter Font Colour', '');
	if (fCol!=null) {
		document.getElementById("wmeditor").contentWindow.document.execCommand('ForeColor', false, fCol);
	}
}

function SetFont(what, how)
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById("wmeditor").contentWindow.document.execCommand(what, false, how);
}

function selOn(ctrl)
{
	ctrl.style.borderColor = '#000000';
	ctrl.style.cursor = 'hand';
}

function selOff(ctrl)
{
	ctrl.style.borderColor = '#F0F0F0';
}
