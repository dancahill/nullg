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

function GoTo(url) {
	$.ajax({
		//url: url+'&method=ajax',
		url: url,
		context: document.body
	}).done(function(msg) {
		$('#main').html(msg);
	});
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
function ViewContact(contactid) {
	$.ajax({
		url: "/contacts/view?id="+contactid+"&method=ajax",
		context: document.body
	}).done(function(msg) {
		$('#dialog-form').html(msg);
		dialog = $( "#dialog-form" ).dialog({
			autoOpen: false,
			height: 400,
			width: 600,
			modal: true,
//			show: "blind",
//			hide: "explode",
			buttons: {
//				"Create an account": addUser,
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
		dialog.dialog( "option", "title", "contact" );
		dialog.dialog( "open" );
	});
}

/*
 * called from the email list page
 */
function ViewEmail(emailid) {
	$.ajax({
		url: "/mail/view?id="+emailid+"&part=header&method=ajax",
		context: document.body
	}).done(function(msg) {
		$('#dialog-form').html(msg);
		dialog = $( "#dialog-form" ).dialog({
			autoOpen: false,
			height: $(window).height()*0.96,
			width: $(window).width()*0.96,
			modal: true,
//			show: "blind",
//			hide: "explode",
			buttons: {
//				"Create an account": addUser,
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
		dialog.dialog( "option", "title", "email" );
		dialog.dialog( "open" );
	});
}

/*
	"$('#dialog-form').html(\"<div id='dialog-form' title='Create new user' STYLE='display: block'>",
	"  <p class='validateTips'>All form fields are required.</p>",
	" ",
	"  <form>",
	"    <fieldset>",
	"      <label for='name'>Name</label>",
	"      <input type='text' name='name' id='name' value='Jane Smith' class='text ui-widget-content ui-corner-all'>",
	"      <label for='email'>Email</label>",
	"      <input type='text' name='email' id='email' value='jane@smith.com' class='text ui-widget-content ui-corner-all'>",
	"      <label for='password'>Password</label>",
	"      <input type='password' name='password' id='password' value='xxxxxxx' class='text ui-widget-content ui-corner-all'>",
	" ",
	"      <!-- Allow form submission with keyboard without duplicating the dialog button -->",
	"      <input type='submit' tabindex='-1' style='position:absolute; top:-1000px'>",
	"    </fieldset>",
	"  </form>",
	"</div>\");\r\n",
*/
