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
	if (bName=='Netscape') {
		var bVer=useragent.substring(8);
		var pos=bVer.indexOf(' ');
		var bVer=bVer.substring(0, pos);
	}
	if (bName=='Netscape'&&parseInt(navigator.appVersion)>=5) {
		var pos=useragent.lastIndexOf('/');
		var bVer=useragent.substring(pos+1);
	}
	return (new Array(bName, bVer));
}
var user_agent=browser_version();
var BrowserSupportsHTMLEditor;

BrowserSupportsHTMLEditor=(user_agent[0]=='Microsoft Internet Explorer'&&user_agent[1]>=4)?true:false;
BrowserSupportsHTMLEditor=(user_agent[0]=='Netscape'&&user_agent[1]>=20030312)?true:BrowserSupportsHTMLEditor;
EditorMode='PLAIN';

function init()
{
	if (!BrowserSupportsHTMLEditor) return;
	document.wmcompose.ctype.value='plain';
	if (user_agent[0]=='Microsoft Internet Explorer') {
		document.getElementById('wmeditor').contentWindow.document.designMode='on';
	}
}

function toggle_mode()
{
	var editor;

	if (!BrowserSupportsHTMLEditor) return false;
	if (document.wmcompose.ctype.value=='plain') {
		editor=document.getElementById('wmeditor').contentWindow.document;
		EditorMode='PLAIN';
		if (user_agent[0]=='Microsoft Internet Explorer') {
			document.wmcompose.msgbody.value=editor.body.innerText;
		} else {
			document.wmcompose.msgbody.value=editor.body.innerHTML;
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/<br>/gi, '\r\n');
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/&lt;/gi, '<');
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/&gt;/gi, '>');
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/&nbsp;/gi, ' ');
		}
		editor.body.style.fontFamily='courier';
		editor.body.style.fontSize='9pt';
		if (user_agent[0]=='Netscape') {
			document.getElementById('wmeditor').contentWindow.document.designMode='off';
		}
		document.getElementById('MenuBar').style.display='none';
		document.wmcompose.msgbody.style.display='inline';
		document.wmcompose.msgbody.focus();
	} else {
		EditorMode='HTML';
		document.getElementById('MenuBar').style.display='block';
		if (user_agent[0]=='Netscape') {
			document.getElementById('wmeditor').contentWindow.document.designMode='on';
		}
		editor=document.getElementById('wmeditor').contentWindow.document;
		if (user_agent[0]=='Microsoft Internet Explorer') {
			editor.body.innerText=document.wmcompose.msgbody.value;
		} else {
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/</gi,'&lt;');
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/>/gi,'&gt;');
			document.wmcompose.msgbody.value=document.wmcompose.msgbody.value.replace(/\n/gi,'<BR>');
			editor.body.innerHTML=document.wmcompose.msgbody.value;
		}
		document.wmcompose.msgbody.style.display='none';
		editor.body.style.fontFamily='';
		editor.body.style.fontSize='';
		document.getElementById('wmeditor').contentWindow.focus();
}	}

function copy_submit()
{
	if (!BrowserSupportsHTMLEditor) return false;
	if (EditorMode=='HTML') {
		document.wmcompose.msgbody.value=document.getElementById('wmeditor').contentWindow.document.body.innerHTML;
	}
}

function wmformat(what)
{
	if (!BrowserSupportsHTMLEditor) return;
	if (EditorMode!='HTML') return;
	document.getElementById('wmeditor').contentWindow.document.execCommand(what, false, null);
	document.getElementById('wmeditor').contentWindow.focus();
}

function html_action(what)
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById('wmeditor').contentWindow.document.execCommand(what);
}

function doImage()
{
	var imgSrc;

	if (!BrowserSupportsHTMLEditor) return;
	imgSrc=prompt('Enter Image URL', '');
	if (imgSrc!=null) {
		document.getElementById('wmeditor').contentWindow.document.execCommand('InsertImage', false, imgSrc);
	}
}

function doLink()
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById('wmeditor').contentWindow.document.execCommand('CreateLink', false, null);
}

function SetForeColour()
{
	var fCol;

	if (!BrowserSupportsHTMLEditor) return;
	fCol=prompt('Enter Font Colour', '');
	if (fCol!=null) {
		document.getElementById('wmeditor').contentWindow.document.execCommand('ForeColor', false, fCol);
	}
}

function SetFont(what, how)
{
	if (!BrowserSupportsHTMLEditor) return;
	document.getElementById('wmeditor').contentWindow.document.execCommand(what, false, how);
}
