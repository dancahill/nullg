function ShowHelp()
{
	var url='/help/en/index.html';
/*
	if (menu=='admin')          print("ch-02.html");
	else if (menu=='bookmarks') print("ch-03.html");
	else if (menu=='calendar')  print("ch-04.html");
	else if (menu=='calls')     print("ch-05.html");
	else if (menu=='contacts')  print("ch-06.html");
	else if (menu=='email')     print("ch-07.html");
	else if (menu=='files')     print("ch-08.html");
	else if (menu=='forums')    print("ch-09.html");
	else if (menu=='invoices')  print("ch-12.html");
	else if (menu=='notes')     print("ch-11.html");
	else if (menu=='profile')   print("ch-13.html");
	else if (menu=='searches')  print("ch-14.html");
	else if (menu=='tasks')     print("ch-04.html");
	else if (menu=='weblog')    print("index.html");
	else                        print("index.html");
*/
//	window.open('/help/en/index.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=780,height=550');
	window.open(url,'helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=780,height=550');
}

/*
function MsgTo(msg) {
	window.open('/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');
}

function ComposeMail() {
	window.open('/mail/write','_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');
}

function MsgTo(msg) {
	window.open('/mail/write?to='+msg,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');
}

//if (typeof(_GET['LOCATION'])!='null') p=_GET['LOCATION']; else p=_SERVER['REQUEST_URI'];
function FileUpload() {
	window.open('/fileul?location=",p,"','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=450,height=320');
}

function ListUsers() {
	window.open('/messages/userlist','msguserlist','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=200,height=300');
}
*/

//function GoTo(url) {
function GoTo(event) {
	try {
		//console.log("GoTo(event):",event);
		if (typeof(event)=='string') {
			url=event;
			console.log("GoTo(); received non-event:",event);
		} else {
			if (event.target.href!==undefined) {
				//console.log("event.target.href:",event.target.href);
				url=event.target.href;
			} else {
				// hope it's a picture and the parent 'A' has the href
				//console.log("event.target.parentElement.href:",event.target.parentElement.href);
				url=event.target.parentElement.href;
			}
			//console.log("event:",event);
			//console.log("event.type:",event.type);
			//console.log("event.target:",event.target,"type:",typeof(event.target));
			//console.log("this:",this);
			event.preventDefault();
		}
		$.ajax({
			//url: url+'&method=ajax',
			url: url,
			context: document.body
		}).done(function(msg) {
			if (msg=="authentication required") {
				console.log("authentication required");
				_login.DoLogin();
			} else {
				$('#main').html(msg);
			}
		});
	} catch (e) {
		console.log(e);
	}
	return false;
}

// generic handlers for simpler modals
function ModalForm(event, module, id) {
	try {
		var divname='modal_x';
		var divheight=400;
		var divwidth=600;
		//var url="/unknown";
		var modal=true;

		if (event.target.href!==undefined) {
			var url=event.target.href;
		} else {
			var url=event.target.parentElement.href;
		}
		switch (module) {
		case 'bookmark':
			//var url="/app/bookmarks/editbookmark?id="+id+"&method=ajax";
			var title="Bookmark: "+id;
			break;
		case 'bookmarkfolder':
			//var url="/app/bookmarks/editfolder?id="+id+"&method=ajax";
			var title="Bookmark Folder: "+id;
			break;
		default:
			throw "invalid module "+module;
		}
		var windowwidth=$(window).width()-8;
		if (windowwidth<divwidth) divwidth=windowwidth;
		if (divwidth<400) divwidth=400;
		SetupDialogForm(divname);
		$.ajax({
			url: url,
			context: document.body
		}).done(function(msg) {
			$('#'+divname).html(msg);
			dialog = $( "#"+divname ).dialog({
				autoOpen: false,
				height: divheight,
				width: divwidth,
				minWidth: divwidth,
				maxWidth: divwidth,
				modal: modal,
				buttons: {
				},
				confirm: function() {
					$(this).dialog("close");
				},
				close: function() {
				}
			});
			dialog.dialog( "option", "title", title );
			dialog.dialog( "open" );
		});
	} catch (e) {
		console.log(e);
	}
	return false;
}

function ModalSubmit(event, module, action)
{
	try {
		var divname='modal_x';
		switch (module) {
		case 'bookmark':
			var formname='bookmarkedit';
			var formURL="/app/bookmarks/editbookmark";
			break;
		case 'bookmarkfolder':
			var formname='bookmarkfolderedit';
			var formURL="/app/bookmarks/editfolder";
			break;
		case 'bookmarksearch':
			var formname='bookmarksearch';
			var formURL="/app/bookmarks/search";
			//$('#main').html(msg);
			var divname='main';
			break;
		default:
			throw "invalid module "+module;
		}
		$('input[name=\"submitaction\"]').val(action);
		$.ajax(
		{
			url : formURL,
			type: 'POST',
			data : $( '#'+formname ).serialize()
		}).done(function(msg) {
			$('#'+divname).html(msg);
		});
		event.preventDefault();
		if (event.unbind!==undefined) event.unbind();
	} catch (e) {
		console.log(e);
	}
	return false;
}




function SetupDialogForm(id) {
	if ($('#'+id).length==0) {
		$(document.body).append('<div id='+id+'></div>');
	} else {
		$('#'+id).html('<div id='+id+'></div>');
	}
}

/*
 * called from the contact list page
 */

contacts = {
	view: function (contactid) {
//		var divname='contact_'+contactid;
		var divname='contact_x';
		SetupDialogForm(divname);
		$.ajax({
			url: "/app/contacts/view?id="+contactid+"&method=ajax",
			context: document.body
		}).done(function(msg) {
			$('#'+divname).html(msg);
			var divwidth=600;
			var windowwidth=$(window).width()-8;
			if (windowwidth<divwidth) divwidth=windowwidth;
			if (divwidth<400) divwidth=400;
			dialog = $( "#"+divname ).dialog({
				autoOpen: false,
				height: 400,
				width: divwidth,
				minWidth: divwidth,
				maxWidth: 600,
				modal: false,
				buttons: {
				},
				confirm: function() {
					$(this).dialog("close");
				},
				close: function() {
				}
			});
			dialog.dialog( "option", "title", "contact: "+contactid );
			dialog.dialog( "open" );
		});
	}
};

mail = {
	/*
	 * called from the email list page
	 */
	view: function (emailid) {
		// check the current row and uncheck all others before opening the email
		var table = document.getElementById("contenttable");
		for (i=0; i<table.rows.length; i++) {
			row = table.rows[i];
			var inputs = row.cells[0].getElementsByTagName('input');
			for (j=0; j<inputs.length; j++) {
				checkbox=inputs[j];
				if (checkbox.type != 'checkbox') continue;
				checkbox.checked=(checkbox.name==emailid)?true:false;
			}
		}
		$.ajax({
			url: "/app/mail/view?id="+emailid+"&part=header&method=ajax",
			context: document.body
		}).done(function(msg) {
			$('#dialog-form').html(msg);
			dialog = $( "#dialog-form" ).dialog({
				autoOpen: false,
				height: $(window).height()*0.98,
				width: $(window).width()*0.98,
				modal: true,
	//			show: "blind",
	//			hide: "explode",
				buttons: {
				},
				confirm: function() {
					$(this).dialog("close");
				},
				close: function() {
				}
			});
			dialog.dialog( "option", "title", "email" );
			dialog.dialog( "open" );
		});
	},
	compose: function (emailid) {
		$.ajax({
			url: "/app/mail/compose?id="+emailid+"&part=header&method=ajax",
			context: document.body
		}).done(function(msg) {
			$('#dialog-form').html(msg);
			dialog = $( "#dialog-form" ).dialog({
				autoOpen: false,
				height: $(window).height()*0.98,
				width: $(window).width()*0.98,
				modal: true,
	//			show: "blind",
	//			hide: "explode",
				buttons: {
					"Send": function() {
						var wmcompose=document.getElementById('wmcompose');
						// Firefox doesn't have innerText.  Use textContent.
						var from = document.getElementById("msgfrom");
						wmcompose.msgfrom.value   =from.options[from.selectedIndex].value;
						wmcompose.msgto.value     =document.getElementById('msgto').textContent;
						wmcompose.msgcc.value     =document.getElementById('msgcc').textContent;
						wmcompose.msgbcc.value    =document.getElementById('msgbcc').textContent;
						wmcompose.msgsubject.value=document.getElementById('msgsubject').textContent;
						var iframe=document.getElementById('contentiframe').contentWindow.document;
	//					console.log("msgbody 1="+iframe);
						if (iframe.body==null) alert('contentiframe body is null');
						wmcompose.msgbody.value=iframe.body.innerHTML;
						mail.SubmitSave();
					}
	//				Cancel: function() {
	//					$("#dialog-form").dialog( "close" );
	//				}
				},
				confirm: function() {
					$(this).dialog("close");
				},
				close: function() {
	//				form[ 0 ].reset();
	//				allFields.removeClass( "ui-state-error" );
				}
			});
			dialog.dialog( "option", "title", "new email" );
			dialog.dialog( "open" );
		});
		return false;
	},
	set_list_submit: function () {
		//callback handler for form submit
		$('#webmail').submit(function(e)
		{
		//	var postData = $(this).serializeArray();
			//var postData = $(this).serialize();
			var formURL = $(this).attr('action');
			$.ajax(
			{
				url : formURL,
				type: 'POST',
				//data : postData,
				data : $( '#webmail' ).serialize(),
				success:function(data, textStatus, jqXHR) 
				{
					//data: return data from server
					if (data=='success') {
						//alert('It worked.');
						GoTo('/app/mail/list');
					} else {
						alert(data);
					}
				},
				error: function(jqXHR, textStatus, errorThrown) 
				{
					alert('error');
					//if fails      
				}
			});
			e.preventDefault(); //STOP default action
			if (e.unbind!==undefined) e.unbind(); //unbind. to stop multiple form submit.
		});
	},
	set_composer_submit: function () {
		//callback handler for form submit
		$('#wmcompose').submit(function(e)
		{
			var formData = new FormData();
			var form=document.getElementById('wmcompose');
			formData.append('inreplyto',  form.inreplyto.value);
		//	var from = document.getElementById('msgfrom');
		//	formData.append('msgfrom',    from.options[from.selectedIndex].value);
			formData.append('msgfrom',    form.msgfrom.value);
			formData.append('msgto',      form.msgto.value);
			formData.append('msgcc',      form.msgcc.value);
			formData.append('msgbcc',     form.msgbcc.value);
			formData.append('msgsubject', form.msgsubject.value);
			formData.append('msgbody',    form.msgbody.value);
			//formData.append('file',content[1].files[0]);
			if (typeof($('input[name=fattach]')[0].files[0]) !== 'undefined') {
		//		formData.append('fattach',    form.fattach);
		//		formData.append('fattach', $('input[name=fattach]')[0].files[0]);
				formData.append('fattach', $('input[name=fattach]')[0].files[0]);
			}
			//m_data.append( 'file_attach', $('input[name=file_attach]')[0].files[0]);
		//	var postData = $(this).serializeArray();
			//var postData = $(this).serialize();
			var formURL = $(this).attr('action');
			var success=false;
			$.ajax(
			{
				url : formURL,
				type: 'POST',
				contentType: false,
				data: formData,
				processData: false,
				context: document.body,
				success:function(data, textStatus, jqXHR) 
				{
					if (data=='success') {
						alert('Success: Message saved in Outbox.');
						$('#dialog-form').dialog('close');
					} else {
						alert('Error?: '+data);
					}
				},
				error: function(jqXHR, textStatus, errorThrown) 
				{
					alert('error');
				}
			});
			e.preventDefault(); //STOP default action
			if (e.unbind!==undefined) e.unbind(); //unbind. to stop multiple form submit.
		});
	},
	SubmitSave: function ()
	{
		mail.set_composer_submit();
		$('input[name=\"submitaction\"]').val('save');
		$('#wmcompose').submit(); //Submit  the FORM
	},
	SubmitDraft: function ()
	{
		mail.set_composer_submit();
		$('input[name=\"submitaction\"]').val('draft');
		$('#wmcompose').submit(); //Submit  the FORM
	},
	load_view_body: function (headerid) {
		$.ajax({
			url: "/app/mail/view?id="+headerid+"&part=body&method=ajax",
			context: document.body
		}).done(function(msg) {
			iframe = document.getElementById('contentiframe');
			if (iframe.addEventListener) {
				//console.log("iframe.addEventListener");
				iframe.addEventListener('load', function load() {
					document.getElementById('contentiframe').removeEventListener('load', load, false);
					//console.log("C: Local iframe is now loaded.");
					//console.log("$('#contentiframe').contents().height() = " + $('#contentiframe').contents().height());
					//console.log("iframe.height = " + iframe.height);
					iframe.height = $('#contentiframe').contents().height();
				}, false);
			} else if (iframe.attachEvent) {
				//console.log("iframe.attachEvent");
				iframe.attachEvent("onload", function() {
					//console.log("A: Local iframe is now loaded.");
					//console.log("$('#contentiframe').contents().height() = " + $('#contentiframe').contents().height());
					//console.log("iframe.height = " + iframe.height);
					iframe.height = $('#contentiframe').contents().height();
				});
			} else {
				//console.log("iframe.onload");
				iframe.onload = function() {
					//console.log("B: Local iframe is now loaded.");
					//console.log("$('#contentiframe').contents().height() = " + $('#contentiframe').contents().height());
					//console.log("iframe.height = " + iframe.height);
					iframe.height = $('#contentiframe').contents().height();
				};
			}

			iframe.seamless = true;
			//iframe.contentWindow.document.body.innerHTML = msg;
			//console.log("iframe.srcdoc=["+iframe.srcdoc+"]")
			if (iframe.srcdoc===undefined) {
				// I.E.?
				//iframe.contentWindow.document.body.innerHTML = msg;
				var iframeDocument = iframe.contentWindow.document;
				//var content = '<html></html>';
				iframeDocument.open('text/html', 'replace');
				iframeDocument.write(msg);
				iframeDocument.close();
			} else {
				iframe.srcdoc = msg;
			}
			// print("document.getElementById('contentiframe').addEventListener('load', function load() { document.getElementById('contentiframe').removeEventListener('load', load, false); mail.load_view_body("+headerid+");}, false);\r\n");

			//console.log("$('#contentiframe').contents().height() = " + $('#contentiframe').contents().height());
			//console.log("iframe.height = " + iframe.height);




			//iframe.height=iframe.contentWindow.document.body.scrollHeight;
	//		iframe.height=$('#contentiframe').contents().height();
	//		$('#contentiframe').contents().find('body').html('');
	//		$('#contentiframe').contents().find('body').append(msg);
	//		iframe.height=iframe.contentWindow.document.body.scrollHeight;
	//		$('#contentiframe').height($('#contentiframe').contents().find('html').height());
	//		$('#contentiframe').height($('#contentiframe').contents().find('body').attr('scrollHeight'));
			});
	},
	load_compose_body: function (headerid) {
		$.ajax({
			url: "/app/mail/compose?id="+headerid+"&part=body&method=ajax",
			context: document.body
		}).done(function(msg) {
			iframe=document.getElementById('contentiframe');
			iframe.contentWindow.document.body.innerHTML = msg;
			//iframe.height=iframe.contentWindow.document.html.body.scrollHeight;
			document.getElementById('contentiframe').contentWindow.document.designMode='on';
		});
	}
/*
<script language="JavaScript">
<!--
function autoResize(id){
    var newheight;
    var newwidth;

    if(document.getElementById){
        newheight=document.getElementById(id).contentWindow.document .body.scrollHeight;
        newwidth=document.getElementById(id).contentWindow.document .body.scrollWidth;
    }

    document.getElementById(id).height= (newheight) + "px";
    document.getElementById(id).width= (newwidth) + "px";
}
//-->
</script>
*/
}

notes = {
	/*
	 * called from the contact list page
	 */
	view: function (noteid) {
		SetupDialogForm('dialog-form');
		$.ajax({
			url: "/app/notes/view?id="+noteid+"&method=ajax",
			context: document.body
		}).done(function(msg) {
			$('#dialog-form').html(msg);
			dialog = $( "#dialog-form" ).dialog({
				autoOpen: false,
				height: $(window).height()*0.98,
				width: $(window).width()*0.98,
				modal: true,
				buttons: {
				},
				confirm: function() {
					$(this).dialog("close");
				},
				close: function() {
				}
			});
			dialog.dialog( "option", "title", "note" );
			dialog.dialog( "open" );
		});
	},
	set_submit: function () {
		$('#noteedit').submit(function(e)
		{
			var formURL = $(this).attr('action');
			$.ajax(
			{
				url : formURL,
				type: 'POST',
				data : $( '#noteedit' ).serialize(),
				success:function(data, textStatus, jqXHR) 
				{
		//			if (data=='success') {
		//				GoTo('/app/notes/list');
		//			} else {
		//				alert(data);
		//			}
					if (data=='success') {
						alert('Success: Note saved.');
						$('#dialog-form').dialog('close');
					} else {
						alert('Error?: '+data);
					}
				},
				error: function(jqXHR, textStatus, errorThrown) 
				{
					alert('error');
					//if fails      
				}
			});
			e.preventDefault(); //STOP default action
			if (e.unbind!==undefined) e.unbind(); //unbind. to stop multiple form submit.
		});
	},
	SubmitSave: function ()
	{
		$('input[name=\"submitaction\"]').val('save');
		$('#noteedit').submit(); //Submit  the FORM
	}
}

_login = {
	DoLogin: function () {
		//window.onbeforeunload = function() { return 'Leaving the page will cause the application to reload.'; };
		//$('#container').html("<b>Loading...</b>");
		$('#container').html("");
		$.ajax({
			url: "/auth.nsp",
			context: document.body
		}).done(function(msg) {
			$('#container').html(msg);
		});
	},
	DoSubmit: function (event)
	{
//		$('#login').submit(function(e)
//		{
			var postData = $('#login').serialize();
			var formURL = $(this).attr('action');
			$.ajax(
			{
				//url : formURL,
				url: "/auth.nsp",
				type: 'POST',
				data : postData,
				context: document.body
			}).done(function(msg) {
				$('#container').html(msg);
			});
			event.preventDefault();
			if (event.unbind!==undefined) event.unbind(); //unbind. to stop multiple form submit.
//		});
//		$('#login').submit();
		return false;
	}
}


/*
 * array of functions used in the webmail WYSIWYG editor
 */
wmedit = {
	get_iframe: function () {
		return document.getElementById('contentiframe').contentWindow;
	},
	init: function () {
		document.wmcompose.ctype.value='plain';
		document.getElementById('contentiframe').contentWindow.document.designMode='on';
	},
//	copy_submit: function () {
//		if (EditorMode=='HTML') {
//			document.wmcompose.msgbody.value=wmedit.get_iframe().document.body.innerHTML;
//		}
//	},
	wmformat: function (what) {
		wmedit.get_iframe().document.execCommand(what, false, null);
		wmedit.get_iframe().focus();
	},
	html_action: function (what) {
		wmedit.get_iframe().document.execCommand(what);
	},
	doImage: function () {
		var imgSrc=prompt('Enter Image URL', '');
		if (imgSrc!=null) wmedit.get_iframe().document.execCommand('InsertImage', false, imgSrc);
	},
	doLink: function () {
		wmedit.get_iframe().document.execCommand('CreateLink', false, null);
	},
	SetForeColour: function () {
		var fCol=prompt('Enter Font Colour', '');
		if (fCol!=null) wmedit.get_iframe().document.execCommand('ForeColor', false, fCol);
	},
	SetFont: function (what, how) {
		wmedit.get_iframe().document.execCommand(what, false, how);
	},
	selOn: function (ctrl)
	{
		ctrl.style.borderColor = '#000000';
		ctrl.style.cursor = 'hand';
	},
	selOff: function (ctrl)
	{
		ctrl.style.borderColor = '#F0F0F0';
	}
};
