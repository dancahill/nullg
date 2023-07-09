
ns = {
	loadJS: function (url, implementationCode, location) {
		//url is URL of external file, implementationCode is the code
		//to be called from the file, location is the location to
		//insert the <script> element

		var scriptTag = document.createElement('script');
		scriptTag.src = url;

		scriptTag.onload = implementationCode;
		scriptTag.onreadystatechange = implementationCode;

		location.appendChild(scriptTag);
	},
	//var yourCodeToBeCalled = function () {
	//	//your code goes here
	//}
	//loadJS('yourcode.js', yourCodeToBeCalled, document.body);

	// start modules
	contacts: {
		location: {
			MarkerFocus: function (id) {
				var marker = markers[id];
				if (marker === undefined) {
					console.log("marker is undefined");
					return;
				}
				//console.log(marker);
				//timelinemap.panTo(marker.getLatLng());
				//timelinemap.flyTo(marker.getLatLng());
				//timelinemap.setView(marker.getLatLng(), 13);
				timelinemap.setView(marker.getLatLng());
				marker.openPopup();
			},
			reload: function () {
				console.log("reload called");
				//td=sprintf("<td class='r60' onclick=\"return ns.dialog.create('%s');\">", url);
				//"/app/contacts/timeline"

				var div = document.getElementById("dialog_timeline_x");
				//console.log("div=", div);
				if (div != null) {
					var field = div.querySelector('input[name="locationdate"]');
					//console.log("field=", div);
					if (field != null) {
						var d = field.value;
						//ns.dialog.close("dialog_timeline_x");
						$("dialog_timeline_x").dialog("close");
						//$("dialog_timeline_x").dialog("destroy");
						ns.dialog.create('/app/contacts/timeline?id=1&date=' + d);
					}
				}
			},
			list_history: function (rows) {
				if (rows === null) {
					console.log("rows===null - no data to list");
					return;
				}
				//console.log(rows);
				var html = "[[[[blah]]]]";
				var body = "";
				for (var key in rows) {
					row = rows[key];
					rowtext = "";
					rowtext += "<tr id='row" + row.id + "'>";
					rowtext += "<td title='" + ns.HTMLEncode(row.date) + "\n" + ns.HTMLEncode(row.lat) + "," + ns.HTMLEncode(row.lon) + "' onclick='ns.contacts.location.MarkerFocus(" + row.id + ")' style='text-align:right'>" + ns.HTMLEncode(row.date) + "</td>";
					rowtext += "</td>";
					rowtext += "</tr>\r\n";
					body += rowtext;
				}
				if (body.length > 0) {
					html = "<table id=\"contenttable\" name=\"locationlist\" class=\"contenttable\" style=\"width:100%\">" +
						"<thead style=\"width=100%\"><tr>\r\n" +
						"<th>date</th>\r\n" +
						"</tr></thead>\r\n" +
						"<tbody style=\"width=100%;overflow:auto\">\r\n" + body + "</tbody>\r\n" +
						"</table>";
				} else {
					html = "[[[[" + body + "]]]]";
				}
				$('#p1').html(html);

				//$(function() {
				//	$("#contenttable").tablesorter({sortList:[[4,1]]}); //, searchstring==""?3:2
				//});
				//ns.mail.list_resize();
			},

			map_history: function (rows) {
				if (rows === null) {
					console.log("rows===null - no data to map");
					$('#timelinemap').html("<b>No Map Data</b>");
					return;
				}
				var p2 = document.getElementById("timelinemap");
				if (p2 === null) return;


				var rect = p2.getBoundingClientRect();
				//console.log(rect.height);
				//console.log(rect.width);

				var mdx = document.getElementById("dialog_timeline_x");
				//console.log('mdx=', mdx.offsetWidth, ' - ', mdx.offsetHeight);
				//console.log('p2=', p2.offsetWidth, ' - ', p2.offsetHeight);
				//console.log('timelinemap=', $('#timelinemap').width(), ' - ', p2.width);

				document.getElementById("timelinemap").style.width = "" + (p2.offsetWidth - 2) + "px";
				document.getElementById("timelinemap").style.height = "" + (p2.offsetHeight - 2) + "px";

				var latlng = L.latLng(45, -73);

				timelinemap = L.map('timelinemap', {
					attributionControl: false,
					center: latlng,
					zoom: 16
				});//.setView(latlng, 16);

				L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
					attribution: '&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank">OpenStreetMap</a>'
				}).addTo(timelinemap);

				timelinemap.addControl(L.control.attribution({
					position: 'bottomright',
					prefix: '<a href="http://leafletjs.com" title="A JS library for interactive maps" target="_blank">Leaflet</a>'
				}));

				L.control.scale().addTo(timelinemap);

				var polyline = new L.Polyline([]).addTo(timelinemap);
				markers = {};
				bounds = new L.LatLngBounds();
				for (var key in rows) {
					row = rows[key];
					latlng = L.latLng(row['lat'], row['lon']);
					if (latlng.lat === 0 || latlng.lon === 0) {
						console.log("not mapping ", row);
						continue;
					}
					markers[row['id']] = L.marker(latlng).addTo(timelinemap)
						.bindPopup('' + row['date'] + '<br/>' + row['lat'] + ',' + row['lon']);
					//.openPopup();
					bounds.extend(markers[row['id']].getLatLng());
					polyline.addLatLng(latlng);
				}
				//timelinemap.setView(latlng, 16);

				//var group = new L.featureGroup(markers);
				//timelinemap.fitBounds(group.getBounds());
				//timelinemap.fitBounds(polyline.getBounds());

				//var bounds = L.latLngBounds(markersgroup);
				timelinemap.fitBounds(bounds);//works!

				//console.log(markers);

				//https://stackoverflow.com/questions/33468905/draw-lines-between-markers-in-leaflet
				//http://embed.plnkr.co/h7aMwc/

				//			var polyline = new L.Polyline([]).addTo(map);
				//			map.on('click', function(event) {
				//				new L.Marker(event.latlng).addTo(map);
				//				polyline.addLatLng(event.latlng);
				//			});
			},
		},

		emailaddrbook: {
			list: function (field, email) {
				ns.contacts.emailaddrbook.outfield = field;
				var divname = 'emailaddrbook';
				ns.SetupDialogForm(divname);
				$.ajax({
					url: "/app/contacts/addrbook?method=ajax&email=" + email + "",
					context: document.getElementById(divname)
				}).done(function (msg) {
					$('#' + divname).html(msg);
					ns.FixLinks(divname);
					var divwidth = 600;
					var windowwidth = $(window).width() - 8;
					if (windowwidth < divwidth) divwidth = windowwidth;
					//if (divwidth<360) divwidth=360;
					dialog = $("#" + divname).dialog({
						autoOpen: false,
						height: 400,
						width: divwidth,
						minWidth: divwidth,
						maxWidth: 600,
						modal: true,
						buttons: {
						},
						confirm: function () {
							$(this).dialog("close");
						},
						close: function () {
							ns.RemoveDialogForm(divname);
						}
					});
					dialog.dialog("option", "title", "Address Book: " + email);
					dialog.dialog("open");
				});
			},
			add: function (email) {
				var dest = document.getElementById('msg' + ns.contacts.emailaddrbook.outfield);
				if (dest === null) {
					ns.mail.compose(email);
					return;
				}
				var tc = dest.textContent;
				if (tc.indexOf(email) > -1) return;
				if (tc !== "") dest.textContent += ", ";
				dest.textContent += email;
			}
		}
	},
	mail: {
		list_resize: function () {
			try {
				var sethidden = function (row, cell, width) {
					row.cells[cell].style.width = width.toString() + 'px';
					row.cells[cell].style.display = (width > 0) ? "" : "none";
					var divs = row.cells[cell].getElementsByTagName("DIV");
					if (divs.length < 1) return width;
					var div = divs[0];
					if (width === 0) {
						//div.style.width = '0px';
						div.style.visibility = "hidden";
						return width;
					}
					div.style.maxWidth = (width - 18).toString() + 'px';
					//div.style.width = (width - 16).toString() + 'px';
					div.style.visibility = "visible";
					div.style.overflow = "hidden";
					div.style.textOverflow = "ellipsis";

					//div[0].style.overflow = "hidden";
					if (cell === 1 || cell === 3) {
						//row.cells[cell].style.width = w.toString() + 'px';
						//row.cells[cell].style.maxWidth = w.toString() + 'px';
						row.cells[cell].style.maxWidth = width.toString() + 'px';
						row.cells[cell].style.overflow = "hidden";
						row.cells[cell].style.textOverflow = "ellipsis";

					}
					return width;
				}
				var tl = document.getElementsByName("maillist");
				if (tl.length !== 1) return;
				var t = tl[0];
				tr = t.rows[0];
				var h = window.innerHeight;
				var rect = t.tBodies[0].getBoundingClientRect();
				t.style.maxHeight = (h - rect.top - 5).toString() + 'px';
				t.tHead.style.width = "100%";
				t.tBodies[0].style.width = "100%";
				t.tBodies[0].style.maxHeight = (h - rect.top - 5).toString() + 'px';

				ns.status.show("" + window.innerWidth + "x" + window.innerHeight);
				var mm = window.matchMedia("(max-width:600px)")


				for (var i = t.rows.length - 1; i > 0; i--) {
					var w = window.innerWidth;
					//if (i < 1) continue;
					if (mm.matches) { // (max-width:600px)
						w -= sethidden(t.rows[i], 0, 21);
						w -= sethidden(t.rows[i], 2, 0);
						w -= sethidden(t.rows[i], 3, 0);
						w -= sethidden(t.rows[i], 4, 0);
						w -= sethidden(t.rows[i], 5, 0);
						w -= sethidden(t.rows[i], 6, 0);
						sethidden(t.rows[i], 1, w);
					} else {
						w -= sethidden(t.rows[i], 0, 21);
						w -= sethidden(t.rows[i], 1, 0);
						w -= sethidden(t.rows[i], 2, 150);
						w -= sethidden(t.rows[i], 4, 65);
						w -= sethidden(t.rows[i], 5, 40);
						w -= sethidden(t.rows[i], 6, 11);
						sethidden(t.rows[i], 3, w);
					}
				}

				t.rows[0].style.width = t.rows[1].offsetWidth;
				t.rows[0].style.minWidth = t.rows[1].offsetWidth;
				t.rows[0].style.maxWidth = t.rows[1].offsetWidth;

				for (var j = 0; j < 7; j++) {
					var vis = t.rows[1].cells[j].style.display;
					t.rows[0].cells[j].style.display = vis;
					var cws = "0px";
					if (vis !== "none") {
						var cw = t.rows[1].cells[j].offsetWidth;
						// cws = "" + (cw - 5) + "px";
						//console.log("cw for "+j+"=", cw);
						cws = "" + (cw - 1) + "px";
					}
					//t.rows[0].cells[j].style.width = cws;
					t.rows[0].cells[j].style.minWidth = cws;
					t.rows[0].cells[j].style.maxWidth = cws;
					t.rows[0].cells[j].style.padding = 0;
					//t.rows[0].cells[j].style.overflow = "hidden";
				}
			} catch (e) {
				console.log("Exception: " + e);
				if (e.message) console.log("Exception: " + e.message);
				if (e.stack) console.log("Exception: " + e.stack);
			}
		},
		list_print: function (messages) {
			//console.log(messages);
			var html = "[[[[blah]]]]";
			var body = "";
			//console.log(window.location.search);
			for (var key in messages) {
				row = messages[key];
				//console.log(messages[key]);
				rowtext = "";
				rowtext += "<tr id='row" + row.headerid + "' " + (row.status !== "r" ? " class='unread'" : "") + ">";
				rowtext += "<td class='col0'><input type=\"checkbox\" name=\"" + row.headerid + "\" id=\"" + row.headerid + "\" value=\"" + row.uidl + "\"></td>";
				rowtext += "<td class='col1' onClick=\"ns.mail.view(" + row.headerid + ");\"><div style='font-size:18px'><b><a href='javascript:void();'>" + ns.HTMLEncode(row.from.full) + "</a></b> - " + row.date + "<br/>" +
					"<b>" + ns.HTMLEncode(row.subject) + "</b><br/>" +
					"</div></td>";
				rowtext += "<td class='col2' title=\"" + ns.HTMLEncode(row.from.full) + "\"><div>" + ns.HTMLEncode(row.from.name) + "</div></td>";
				rowtext += "<td class='col3' onClick=\"ns.mail.view(" + row.headerid + ");\" title=\"" + ns.HTMLEncode(row.subject) + "\">" +
					"<div><a href=javascript: ns.mail.view(" + row.headerid + "); title=\"" + ns.HTMLEncode(row.subject) + "\">" + ns.HTMLEncode(row.subject) + "</a></div></td>";
				rowtext += "<td class='col4' title=\"" + row.datefull + "\">" + row.date + "</td>";
				rowtext += "<td class='col5' style=\"text-align:right\">" + row.ssize + "</td>";
				rowtext += "<td class='col6'>";
				rowtext += row.attachment ? "<img border=0 src=/lib/images/paperclip.png height=16 width=11 alt='File Attachments'>" : "&nbsp;&nbsp;&nbsp;";
				rowtext += "</td>";
				rowtext += "</tr>\r\n";
				body += rowtext;
			}
			if (body.length > 0) {
				//https://stackoverflow.com/questions/19559197/how-to-make-scrollable-table-with-fixed-headers-using-css
				//var html="<table id=\"contenttable\" class=\"maillist contenttable\" style=\"width:100%;table-layout:fixed;\">"+
				//var html="<table id=\"contenttable\" name=\"maillist\" class=\"maillist contenttable\" style=\"height:100%;width:100%;table-layout:fixed;\">"+
				html = "<table id=\"contenttable\" name=\"maillist\" class=\"maillist contenttable\" style=\"width:100%;table-layout:fixed\">" +
					//"<thead style=\"display:block;\"><tr>\r\n"+
					"<thead style=\"width=100%;display:block;\"><tr>\r\n" +
					"<th class=\"col0 header\">&nbsp;</th>\r\n" +
					"<th class=\"col1 header\"> </th>\r\n" +
					"<th class=\"col2 header\">&nbsp;From&nbsp;</th>\r\n" +
					"<th class=\"col3 header\">&nbsp;Subject&nbsp;</th>\r\n" +
					"<th class=\"col4 header headerSortUp\">&nbsp;Date&nbsp;</th>\r\n" +
					"<th class=\"col5 header\">&nbsp;Size&nbsp;</th>\r\n" +
					"<th class=\"col6 header\">&nbsp;</th>\r\n" +
					"</tr></thead>\r\n" +
					//"<tbody style=\"display:block;overflow:auto;\">\r\n"+body+"</tbody>\r\n"+
					"<tbody style=\"width=100%;display:block;overflow:auto;\">\r\n" + body + "</tbody>\r\n" +
					"</table>";
			} else {
				html = "[[[[" + body + "]]]]";
			}
			$('#messagelist').html(html);
			$(function () {
				$("#contenttable").tablesorter({ sortList: [[4, 1]] }); //, searchstring==""?3:2
			});
			ns.mail.list_resize();
		},

		list: function (p) {
			//console.log("list(",p,")");
			if (p.length > 0) p = "?" + p;
			$.ajax({
				url: "/app/mail/list-json" + p,
			}).done(function (msg) {
				ns.mail.list_print(JSON.parse(msg));
				//ns.FixLinks("main");
			});
		},

		search: function () {
			//console.log("list(",p,")");
			//if (p.length > 0) p = "?" + p;
			$.ajax({
				// url: "/app/mail/list-json" + p,
				url: "/app/mail/list-json",
				type: 'POST',
				data: $('#mailsearch').serialize()
			}).done(function (msg) {
				ns.mail.list_print(JSON.parse(msg));
				//ns.FixLinks("main");
			});
			return false;
		},

		view: function (emailid) {
			var divname = "email_form";
			ns.SetupDialogForm(divname);
			// check the current row and uncheck all others before opening the email
			var table = document.getElementById("contenttable");
			for (var i = 0; i < table.rows.length; i++) {
				var row = table.rows[i];
				var inputs = row.cells[0].getElementsByTagName('input');
				for (var j = 0; j < inputs.length; j++) {
					var checkbox = inputs[j];
					if (checkbox.type !== 'checkbox') continue;
					checkbox.checked = (checkbox.name === String(emailid)) ? true : false;
				}
				var tr = document.getElementById('row' + emailid);
				if (tr !== null) tr.className = ''; // remove 'unread' class from tr
			}
			$.ajax({
				url: "/app/mail/view?id=" + emailid + "&part=header&method=ajax",
				context: document.body
			}).done(function (msg) {
				$('#' + divname).html(msg);
				ns.FixLinks(divname);
				dialog = $("#" + divname).dialog({
					autoOpen: false,
					height: $(window).height() * 0.98,
					width: $(window).width() * 0.98,
					modal: true,
					show: "fade",
					hide: "fade",
					buttons: {
					},
					confirm: function () {
						$(this).dialog("close");
					},
					close: function () {
						ns.RemoveDialogForm(divname);
					}
				});
				dialog.dialog("option", "title", "email");
				dialog.dialog("open");
			});
		},
		/*
			sync: function () {
				$.ajax({
					url: "/app/mail/sync",
					context: document.body
				}).done(function(msg) {
					ns.status.show(msg);
					//$('#container').html(msg);
				});
				return false;
			},
		*/
		getattachment: function (emailid, attachmentname) {
			window.open('/app/mail/attachment.ns?id=' + emailid + '&filename=' + attachmentname)
		},
		delete: function (emailid) {
			if (confirm("Are you sure you want to delete this record?") !== true) return;
			var tr = document.getElementById('row' + emailid);
			if (tr === null) return;
			var formData = new FormData();
			formData.append('submitaction', 'delete');
			formData.append('dest1', '');
			formData.append(emailid, tr.cells[0].getElementsByTagName('input')[0].value);
			$.ajax({
				url: '/app/mail/move',
				type: 'POST',
				contentType: false,
				data: formData,
				processData: false,
				context: document.body,
				success: function (data, textStatus, jqXHR) {
					if (data === 'success') {
						//alert('Success: Message saved in Outbox.');
						document.getElementById("contenttable").deleteRow(tr.rowIndex);
						$('#email_form').dialog('close');
						ns.status.show("email " + emailid + " deleted");
					} else {
						alert('Error?: ' + data);
					}
				},
				error: function (jqXHR, textStatus, errorThrown) {
					alert('error');
				}
			});
		},
		compose: function (emailid) {
			var divname = "email_compose_form";

			ns.RemoveDialogForm("email_form");

			ns.SetupDialogForm(divname);
			var emailaddr = "";
			if (typeof (emailid) === "string") {
				emailaddr = emailid;
				emailid = 0;
			}
			$.ajax({
				url: "/app/mail/compose?id=" + emailid + "&part=header&method=ajax",
				context: document.body
			}).done(function (msg) {
				$('#' + divname).html(msg);
				ns.FixLinks(divname);
				if (emailaddr !== "") {
					ns.contacts.emailaddrbook.outfield = "to";
					ns.contacts.emailaddrbook.add(emailaddr);
				}
				dialog = $("#" + divname).dialog({
					autoOpen: false,
					height: $(window).height() * 0.98,
					width: $(window).width() * 0.98,
					modal: true,
					//	show: "blind",
					//	hide: "explode",
					show: "fade",
					hide: "fade",

					//	buttons: {
					//		"Send": function() {
					//			mail.SubmitSave();
					//		}
					//	},
					confirm: function () {
						$(this).dialog("close");
					},
					close: function () {
						//	form[ 0 ].reset();
						//	allFields.removeClass( "ui-state-error" );
						//	document.getElementById(divname).innerHTML = "";
						ns.RemoveDialogForm(divname);
					}
				});
				dialog.dialog("option", "title", "new email");
				dialog.dialog("open");
			});
			return false;
		},
		set_list_submit: function () {
			//callback handler for form submit
			$('#webmail').submit(function (e) {
				//	var postData = $(this).serializeArray();
				//var postData = $(this).serialize();
				var formURL = $(this).attr('action');
				$.ajax({
					url: formURL,
					type: 'POST',
					//data : postData,
					data: $('#webmail').serialize(),
					success: function (data, textStatus, jqXHR) {
						//data: return data from server
						if (data === 'success') {
							//alert('It worked.');
							ns.GoTo('/app/mail/list');
						} else {
							alert(data);
						}
					},
					error: function (jqXHR, textStatus, errorThrown) {
						alert('error');
						//if fails
					}
				});
				e.preventDefault(); //STOP default action
				if (e.unbind !== undefined) e.unbind(); //unbind. to stop multiple form submit.
			});
		},
		SubmitSave: function (folder) {
			if (folder !== "send" && folder !== "draft") throw "folder invalid [" + folder + "]";
			var iframe = document.getElementById('contentiframe').contentWindow.document;
			if (iframe.body === null) throw 'contentiframe body is null';
			var msgfrom = document.getElementById("msgfrom");
			var formData = new FormData();
			formData.append('folder', folder);
			formData.append('inreplyto', document.getElementById('inreplyto').value);
			formData.append('msgfrom', msgfrom.options[msgfrom.selectedIndex].value);
			// Firefox doesn't have innerText.  Use textContent.
			formData.append('msgto', document.getElementById('msgto').textContent);
			formData.append('msgcc', document.getElementById('msgcc').textContent);
			formData.append('msgbcc', document.getElementById('msgbcc').textContent);
			formData.append('msgsubject', document.getElementById('msgsubject').textContent);
			formData.append('msgbody', iframe.body.innerHTML);
			var files = document.getElementById('fattach').files;
			var maxfilesize = 8 * 1048576;// 8 MB total size limit
			var totalsize = 0;
			for (var i = 0; i < files.length; i++) {
				totalsize += files[i].size;
				if (totalsize > maxfilesize) {
					var mtext = "attachment(s) too large (>" + maxfilesize / 1048576 + " MB)";
					alert(mtext);
					throw mtext;
				}
				formData.append('fattach' + i, files[i]);
			}
			$.ajax({
				xhr: function () {
					var xhr = new window.XMLHttpRequest();
					xhr.upload.addEventListener("progress", function (evt) {
						if (evt.lengthComputable) {
							var percentComplete = evt.loaded / evt.total;
							percentComplete = parseInt(percentComplete * 100);
							ns.status.show("Upload " + percentComplete + "% complete");
							if (percentComplete === 100) {
								ns.status.show("Upload complete");
							}
						}
					}, false);
					return xhr;
				},
				url: "/app/mail/save",
				type: 'POST',
				contentType: false,
				data: formData,
				processData: false,
				//context: document.body,
				success: function (data, textStatus, jqXHR) {
					if (data === 'success') {
						$('#email_compose_form').dialog('close');
						ns.status.show("Message saved in Outbox");
					} else {
						alert('Error?: ' + data);
					}
				},
				error: function (jqXHR, textStatus, errorThrown) {
					alert('error');
				}
			});
		},
		//	SubmitDraft: function ()
		//	{
		//		ns.mail.set_composer_submit();
		//		$('input[name=\"submitaction\"]').val('draft');
		//		$('#wmcompose').submit(); //Submit  the FORM
		//	},
		load_view_body: function (headerid) {
			$.ajax({
				url: "/app/mail/view?id=" + headerid + "&part=body&method=ajax",
				context: document.body
			}).done(function (msg) {
				iframe = document.getElementById('contentiframe');
				if (iframe.addEventListener) {
					iframe.addEventListener('load', function load() {
						document.getElementById('contentiframe').removeEventListener('load', load, false);
						iframe.height = $('#contentiframe').contents().height();
					}, false);
				} else if (iframe.attachEvent) {
					iframe.attachEvent("onload", function () {
						iframe.height = $('#contentiframe').contents().height();
					});
				} else {
					iframe.onload = function () {
						iframe.height = $('#contentiframe').contents().height();
					};
				}
				iframe.seamless = true;
				//iframe.contentWindow.document.body.innerHTML = msg;
				//console.log("iframe.srcdoc=["+iframe.srcdoc+"]")
				if (iframe.srcdoc === undefined) {
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
				ns.FixLinks("contentiframe");
			});
		},
		load_compose_body: function (headerid) {
			$.ajax({
				url: "/app/mail/compose?id=" + headerid + "&part=body&method=ajax",
				context: document.body
			}).done(function (msg) {
				iframe = document.getElementById('contentiframe');
				iframe.contentWindow.document.body.innerHTML = msg;
				//iframe.height=iframe.contentWindow.document.html.body.scrollHeight;
				document.getElementById('contentiframe').contentWindow.document.designMode = 'on';
			});
		},
		accountview(event, id) {
			ns.dialog.create(event, 'mailaccount', id);
		}
	},
	// finish modules






	status: {
		check: function () {
			setTimeout(function () { ns.status.check(); }, 60000);
			if (!document.getElementById("main")) {
				//console.log("main element NOT found");
				return;
			}
			//console.log("main element found");
			$.ajax({
				url: "/app/status",
				context: document.body
			}).done(function (msg) {
				// if (msg === "authentication required") {
				// 	console.log("authentication required");
				// 	ns.status.show("you have been logged out");
				// 	ns.login.DoLogin();
				// } else
				if (msg !== "") {
					ns.status.show(msg);
					var audio = new Audio('/lib/sounds/reminder.wav');
					audio.play();
				}
			}).fail(function (jqXHR, textStatus) {
				if (jqXHR.status === 403) {
					ns.login.DoLogin();
				}
			});
		},
		show: function (message, delay) {
			if (delay === undefined) {
				delay = 30;
			}
			var div = document.getElementById("status_popup");
			if (div === null) {
				div = document.createElement('div');
				div.id = 'status_popup';
				div.style.position = "fixed";
				div.style.bottom = '0';
				div.style.right = '0';
				div.style.backgroundColor = '#C0C0C0';
				//div.className = 'ui-modal';
				div.style.zIndex = 1000;
				document.body.appendChild(div);
			}
			while (message.substring(message.length - 4, message.length) === '<br>') {
				message = message.substring(0, message.length - 4);
			}
			div.innerHTML = "<span class=msg>" + message + "</span>";
			setTimeout(function () { ns.status.hide(); }, delay * 1000);
		},
		hide: function () {
			var div = document.getElementById("status_popup");
			//div.innerHTML = "<span class=msg>goodbye</span>";
			if (div !== null) div.parentNode.removeChild(div);
		}
	},

	login: {
		CloseDivs: function () {
			var x = document.getElementsByClassName("ui-dialog-content");
			var i;
			for (i = 0; i < x.length; i++) {
				try {
					ns.RemoveDialogForm(x[i].id);
				} catch (ex) {
					console.log(ex);
				}
			}
		},
		PrepForm: function () {
			ns.login.CloseDivs();
			window.onbeforeunload = function () { };
			//if (self!=parent) open('/','_top');
			document.login.setAttribute('autocomplete', 'off');
			document.login.username.focus();
			if (window.location.protocol !== 'https:') {
				console.log('Warning: This site is not secure.');
				var span = document.getElementById("warning");
				if (span !== null) span.innerHTML = "<b>Warning: This site is not secure.</b>";
			}
			if (typeof _null !== 'undefined') _null.fillLogin();
		},
		DoLogin: function () {
			//window.onbeforeunload = function() { return 'Leaving the page will cause the application to reload.'; };
			//$('#container').html("<b>Loading...</b>");
			$('#container').html("");
			$.ajax({
				url: "/auth.nsp",
				context: document.body
			}).done(function (msg) {
				$('#container').html(msg);
				ns.FixLinks("container");
			});
		},
		DoSubmit: function (event) {
			var postData = $('#login').serialize();
			var formURL = $(this).attr('action');
			$.ajax({
				//url : formURL,
				url: "/auth.nsp",
				type: 'POST',
				data: postData,
				context: document.body
			}).done(function (msg) {
				$('#container').html(msg);
				ns.FixLinks("container");
			});
			event.preventDefault();
			if (event.unbind !== undefined) event.unbind(); //unbind. to stop multiple form submit.
			return false;
		}
	},

	HTMLEncode: function (value) {
		//https://stackoverflow.com/questions/1219860/html-encoding-lost-when-attribute-read-from-input-field
		//return document.createElement( 'a' ).appendChild( document.createTextNode( value ) ).parentNode.innerHTML.replace(/"/g, '&quot;');
		return $('<div/>').text(value).html().replace(/"/g, '&quot;');
	},
	HTMLDecode: function (value) {
		return $('<div/>').html(value).text();
	},
	ResizeEvents: function () {
		ns.mail.list_resize();
	},
	ShowHelp: function () {
		var url = '/lib/help/en/index.html';
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
		window.open(url, 'helpwin', 'toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=780,height=550');
	},
	showtab: function (event, page) {
		try {
			var id = null;
			var node = null;
			if (event === null) {
				console.log("event===null, this=", this, ".");
			} else if (typeof event === 'string') {
				id = event;
				//console.log("string id=",id,".");
				node = document.getElementById(id);
			} else if (typeof event === 'object') {
				event.preventDefault();
				id = ns.dialog.GetID(event);
				//console.log("id=",id,".");
				node = document.getElementById(id);
			} else {
				console.log("typeof event=", typeof event, ".");
			}
			if (node === null) {
				//node=document;
				console.log("showtab() no node found", event);
				return false;
			}
			var tabs = node.getElementsByClassName("tab");
			var pages = node.getElementsByClassName("tabpage");
			for (var i = 0; i < tabs.length; i++) {
				if (i == page - 1) {
					tabs[i].style.borderBottom = 'solid 0px #000000';
					tabs[i].bgColor = '#F0F0F0';
					pages[i].style.display = 'block';
					//////////////////
					var field = pages[i].querySelector('input');
					if (field == null) field = pages[i].querySelector('textarea');
					if (field != null) field.focus();
				} else {
					tabs[i].style.borderBottom = 'solid 1px #000000';
					tabs[i].bgColor = '#E0E0E0';
					pages[i].style.display = 'none';
				}
			}
			try {
				//console.log("event = ["+event.target.innerText+"]", event.target);
				// this is an ugly hack
				if (event.target.innerText == "CHARTS") {
					setTimeout(function () {
						drawCharts();
					}, 25);
				}
			} catch (ex) {
				//console.log("showtab() Exception: " + ex);
			}
		} catch (ex) {
			console.log("showtab() Exception: " + ex);
		}
		return false;
	},
	focusfield: function (divname, fieldname) {
		setTimeout(function () {
			var div = document.getElementById(divname);
			//console.log("div=", div);
			if (div != null) {
				var field = div.querySelector('input[name="' + fieldname + '"]');
				//console.log("field=", div);
				if (field != null) field.focus();
			}
		}, 50);
	},
	settitle: function (divname, title) {
		setTimeout(function () {
			$("#" + divname).dialog("option", "title", "" + title);
		}, 50);
	},

	FixLinks: function (divname) {
		var div = document.getElementById(divname);
		if (div == null) {
			console.log("FixLinks() can't find divname ", divname);
			return;
		}
		var anchors = div.getElementsByTagName("a");
		for (var i = 0; i < anchors.length; i++) {
			var anchor = anchors[i];
			//var classList = anchor.className.split(' ');
			//console.log("anchor classList = ", classList, ".");
			if (anchor.className == "external") {
				//anchors[i].setAttribute('target', '_blank');
				anchors[i].addEventListener("click",
					function (event) {
						event.preventDefault();
						event.stopPropagation();
						//console.log("external link event=",this.href,"");
						window.open(this.href, '_blank');
					},
					false);
			} else if (anchor.className == "main") {
				anchors[i].addEventListener("click",
					function (event) {
						event.preventDefault();
						event.stopPropagation();
						//console.log("main link event=",this.href,"");
						ns.GoTo(this.href);
					},
					false);
			} else if (anchor.className == "dialog") {
				anchors[i].addEventListener("click",
					function (event) {
						event.preventDefault();
						event.stopPropagation();
						//console.log("dialog link event=",this.href,"");
						ns.dialog.create(event);
					},
					false);
			} else {
				anchors[i].addEventListener("click",
					function (event) {
						//console.log("doing nothing for ",this.href,"");
					},
					false);
			}
		}
	},

	LastGoTo: "",
	GoTo: function (event) {
		try {
			if (typeof (event) === 'string') {
				url = event;
				//console.log("ns.GoTo(); received non-event:",event);
			} else {
				if (event.target.href !== undefined) {
					url = event.target.href;
				} else {
					// hope it's a picture and the parent 'A' has the href
					url = event.target.parentElement.href;
				}
				event.preventDefault();
			}
			$.ajax({
				//url: url+'&method=ajax',
				url: url,
				context: document.body
			}).done(function (msg) {
				ns.LastGoTo = url;
				//console.log("ns.LastGoTo=[" + ns.LastGoTo + "]");
				$('#main').html(msg);
				ns.FixLinks("main");
			}).fail(function (jqXHR, textStatus) {
				if (jqXHR.status === 403) {
					ns.login.DoLogin();
				}
			});
		} catch (e) {
			console.log(e);
		}
		return false;
	},
	ReGoTo: function () {
		if (ns.LastGoTo == "") {
			console.log("ns.LastGoTo=" + ns.LastGoTo + ".");
			ns.LastGoTo = "/app/motd";
			console.log("ns.LastGoTo=" + ns.LastGoTo + ".");
		}
		ns.GoTo(ns.LastGoTo);
	},

	dialog: {
		openers: {},
		setup: function (divname, title, tab) {
			setTimeout(function () {
				var div = document.getElementById(divname);
				if (div == null) return;
				$("#" + divname).dialog("option", "title", "" + title);
				if (tab > 0) ns.showtab(divname, tab);
				// if (fieldname!="") {
				// 	var field=div.querySelector('input[name="'+fieldname+'"]');
				// 	if (field!=null) field.focus();
				// }
			}, 50);
		},
		// generic handlers for simpler dialogs
		// was ns.ModalForm
		create: function (event, targeturl) {//, module, id
			try {
				var autoopen = false;
				var divname = 'dialog_x';
				var divheight = 400;
				var divwidth = 600;
				//var url="/unknown";
				var modal = false;

				var context = {};

				//console.log("typeof event=[",typeof event,"],event=[",event,"]");
				if (event !== undefined) {
					if (typeof targeturl === 'string') {
						var url = targeturl;
					} else if (typeof event === 'string') {
						var url = event;
					} else if (event.target.href !== undefined) {
						var url = event.target.href;
					} else if (event.target.parentElement.href !== undefined) {
						var url = event.target.parentElement.href;
					}
				}
				var request = function () {
					//URI = scheme:[//authority]path[?query][#fragment]
					var x = url.replace(location.origin, "").split("?");
					var result = { path: x[0], id: 0 };
					if (x.length > 1) {
						x[1].split("&").forEach(function (value) {
							nv = value.split("=");
							n = nv[0];
							v = nv[1];
							if (n == "id") v = parseInt(v);
							result[n] = v;
						});
					}
					return result;
				}();
				//console.log("dialog.create() request",request,"");

				var x = request.path.split('/');
				var y = x[x.length - 1];
				divname = "dialog_" + y + "_" + request.id;
				//console.log("divname = '"+divname+"'");
				//console.log("request.path = '"+request.path+"'");
				switch (request.path) {
					case "/app/admin/domainalias":
					case "/app/admin/groupuser":
					case "/app/admin/usergroup":
						var divheight = 120;
						var divwidth = 300;
						modal = true;
						break;
					case "/app/contacts/timeline":
						var divheight = $(window).height() * 0.98;
						var divwidth = $(window).width() * 0.98;
						var divname = 'dialog_timeline_x';
						break;
					case '/app/notes/note':
						var divheight = $(window).height() * 0.98;
						var divwidth = $(window).width() * 0.98;
						//var divname = 'dialog_note_x';
						divheight = divheight > 800 ? 800 : divheight;
						divwidth = divwidth > 800 ? 800 : divwidth;
						break;
					case "/app/projects/project":
						var divheight = $(window).height() * 0.98;
						var divwidth = $(window).width() * 0.98;
						//var divname = 'dialog_timeline_x';
						break;
					default:
						break;
				}

				var windowwidth = $(window).width() - 8;
				if (windowwidth < divwidth) divwidth = windowwidth;
				//if (divwidth<360) divwidth=360;
				ns.SetupDialogForm(divname);

				var parentdiv = ns.dialog.GetID(event);
				if (typeof parentid === 'string') {
					var parentdiv = parentid;
				}
				// console.debug("parentdiv=[",parentdiv,"]");
				// console.log("ns.dialog.openers=", ns.dialog.openers);
				if (parentdiv !== undefined) ns.dialog.openers[divname] = { parentid: parentdiv };
				// console.log("ns.dialog.openers=", ns.dialog.openers);
				$.ajax({
					// beforeSend: function(request) {
					// 	if (typeof parentdiv == 'string') {
					// 		request.setRequestHeader("ParentFormID", parentdiv);
					// 	}
					// },
					// headers: {
					// 	'ParentFormID':parentdiv
					// },
					url: url,
					//context: document.body
					context: context
				}).done(function (msg) {
					$('#' + divname).html(msg);
					ns.FixLinks(divname);
					dialog = $("#" + divname).dialog({
						autoOpen: autoopen,
						height: divheight,
						width: divwidth,
						//minWidth: divwidth,
						//maxWidth: divwidth,
						modal: modal,
						show: "fade",
						hide: "fade",
						buttons: {
						},
						confirm: function () {
							$(this).dialog("close");
						},
						close: function () {
							ns.RemoveDialogForm(divname);
						}
					});
					//dialog.dialog("option", "title", title);
					dialog.dialog("option", "title", url);
					dialog.dialog("open");
				});
			} catch (e) {
				console.log(e);
			}
			return false;
		},
		// was ns.ModalSubmit
		submit: function (event, module, action) {
			event.preventDefault();
			var form = event.target;
			if (form.tagName != "FORM") {
				//console.log("event.target is not a form?\n[event:",event," module:",module," action:",action,"]");
				var form = event.target.form;
				if (form.tagName != "FORM") {
					console.log("event.target and event.target.form are NOT FORMS\n[event:", event, " module:", module, " action:", action, "]");
					return;
				}
			}
			var submitaction = form.querySelector('input[name="submitaction"]');
			if (submitaction == null) {
				console.log("submitaction==null?\n[event:", event, " module:", module, " action:", action, "]");
				return;
			}
			if (document.activeElement.name.toLowerCase() != "submit") {
				//console.log("document.activeElement.name is not submit?\n[event:",event," module:",module," action:",action,"]");
				submitaction.value = 'save';
			} else {
				submitaction.value = document.activeElement.value.toLowerCase();
			}
			var formURL = form.action;
			var id = parseInt(form.querySelector('input[type="hidden"]').value);
			//console.log("id[",id,"],event[",event,"],form[",form,"]");
			// var x=formURL.split('/');
			// var y=x[x.length-1];
			// divname="dialog_"+y+"_"+id;
			// console.log("divname=", divname);
			//console.log("form = ", form);
			//$('input[name=\"submitaction\"]').val(action);
			//var submitaction=form.querySelector('input[name="submitaction"]');
			//console.log("form.submitaction = [", submitaction, "]");
			//console.log("submitaction.value = [", submitaction.value, "]");
			try {
				var divname = 'dialog_' + module + '_' + id;
				var moduleshowname = module;
				switch (module) {
					// case 'domain':
					// 	var formname = 'domainedit';
					// 	var formURL = "/app/admin/domain";
					// 	break;
					// case 'group':
					// 	var formname = 'groupedit';
					// 	var formURL = "/app/admin/group";
					// 	break;
					// case 'user':
					// 	var formname = 'useredit';
					// 	var formURL = "/app/admin/user";
					// 	break;
					// case 'zone':
					// 	var formname = 'zoneedit';
					// 	var formURL = "/app/admin/zone";
					// 	break;
					// case 'bookmark':
					// 	var formname = 'bookmarkedit';
					// 	var formURL = "/app/bookmarks/bookmark";
					// 	break;
					case 'bookmarkfolder':
						// var formname = 'bookmarkfolderedit';
						// var formURL = "/app/bookmarks/bookmarkfolder";
						moduleshowname = "bookmark folder";
						break;
					case 'bookmarksearch':
						// var formname = 'bookmarksearch';
						// var formURL = "/app/bookmarks/list";
						var divname = 'main';
						break;
					// case 'contact':
					// 	var formname = 'contactedit';
					// 	var formURL = "/app/contacts/contact";
					// 	break;
					case 'contactsearch':
						// var formname = 'contactsearch';
						// var formURL = "/app/contacts/list";
						var divname = 'main';
						break;
					// case 'event':
					// 	var formname = 'eventedit';
					// 	var formURL = "/app/calendar/event";
					// 	break;
					case 'mailaccount':
						// var formname = 'mailaccountedit';
						// var formURL = "/app/mail/mailaccounts/account";
						moduleshowname = "mail account";
						break;
					case 'mailsearch':
						// var formname = 'mailsearch';
						// var formURL = "/app/mail/list";
						var divname = 'main';
						break;
					// case 'note':
					// 	var formname = 'noteedit';
					// 	var formURL = "/app/notes/note";
					// 	//var divname = 'dialog_note_x';
					// 	break;
					case 'profile':
						// var formname = 'profileedit';
						// var formURL = "/app/profile/profile";
						var divname = 'main';
						break;
					// case 'project':
					// 	var formname = 'projectedit';
					// 	var formURL = "/app/projects/project";
					// 	break;
					// case 'task':
					// 	var formname = 'taskedit';
					// 	var formURL = "/app/tasks/task";
					// 	break;
					default:
					//throw "invalid module " + module;
				}

				if (submitaction.value === "delete" && confirm("Are you sure you want to delete this " + moduleshowname + "?") !== true) return false;

				$.ajax({
					url: formURL,
					type: 'POST',
					//data: $('#' + formname).serialize()
					data: $(form).serialize()
				}).done(function (msg) {
					$('#' + divname).html(msg);
					ns.FixLinks(divname);
					if (divname != 'main') ns.ReGoTo();
				});
				event.preventDefault();
				if (event.unbind !== undefined) event.unbind();
			} catch (e) {
				console.log(e);
			}
			return false;
		},
		reopen: function (divname, url) {
			setTimeout(function () {
				$('#' + divname).dialog('close');
				ns.dialog.create(url);
			}, 200);
		},
		refreshparent: function (divname, olddivname) {
			//console.log("refresh(",divname,", ",url,")");
			if (divname != olddivname) {
				ns.dialog.openers[divname] = { parentid: ns.dialog.openers[olddivname].parentid };
				// ns.dialog.openers[divname].parentid=ns.dialog.openers[olddivname].parentid;
				// console.log("setting ns.dialog.openers["+divname+"] to ", ns.dialog.openers[divname]);
			}
			// console.log(divname, olddivname, ns.dialog.openers);


			var opener = ns.dialog.openers[divname];
			if (opener === undefined || opener.parentid === undefined) {
				//				opener=ns.dialog.openers[olddivname];
				//				if (opener===undefined || opener.parentid===undefined) {
				console.log("couldn't find opener for " + divname);
				return;
				//				}
				//				ns.dialog.openers[divname]={parentid:opener.parentid};
			}
			var parentid = opener.parentid;
			//console.log(divname+" refreshing parent "+parentid);
			// only reload if it's already open
			var form = document.querySelector('#' + parentid);
			if (form == null) {
				console.log("couldn't find form - maybe it's been closed?");
				return;
			}
			var formurl = form.querySelector('input[name="formurl"]');
			if (formurl == null) {
				console.log("ns.dialog.openers = ", ns.dialog.openers);
				if (parentid == 'main') {
					console.log("couldn't find formurl in form " + divname + ",old=" + olddivname, "parentid=", parentid);
					console.log("loaded from main - ns.GoTo called?", ns.dialog.openers);
				} else {
					console.log("couldn't find formurl in form " + divname + ",old=" + olddivname, "parentid=", parentid);
				}
				return;
			}
			console.log(divname + " refreshing parent " + formurl.value);
			//ns.dialog.create(formurl.value);
			// ns.dialog.create(url);
			$.ajax({
				url: formurl.value
			}).done(function (msg) {
				$('#' + parentid).html(msg);
				ns.FixLinks(parentid);
			});
		},
		close: function (divname) {
			setTimeout(function () {
				$('#' + divname).dialog('close');
			}, 500);
		},
		GetID: function (event) {
			// console.log("event=",event);
			//console.log("event.path=",event.path);
			//console.log("event.srcElement=",event.srcElement);
			parent = event.srcElement;
			while (parent != null && parent !== undefined) {
				if (parent.nodeName != "DIV" || parent.id === undefined || (!parent.id.startsWith('dialog_') && parent.id != 'main')) {
					parent = parent.parentElement;
					continue;
				}
				//console.log("------------------");
				//console.log("parent=",parent);
				//console.log("localName=",parent.localName,"nodeName=",parent.nodeName,"tagName=",parent.tagName,"name=",parent.name);
				//console.log("localName=",parent.localName,"nodeName=",parent.nodeName,"tagName=",parent.tagName,"id=",parent.id,"name=",parent.name);
				//console.log("parent.parentElement=",parent);
				//var node=document.getElementById(parent.id);
				//console.log("node = [",node,"]");
				//console.log("dialog id = [",parent.id,"]");
				return parent.id;
			}
		}
	},



	RemoveDialogForm: function (id) {
		var div = document.getElementById(id);
		if (div === null) return;
		try {
			$('#' + id).dialog("destroy");
		} catch (e) {
			console.log(e);
		}
		div.parentNode.removeChild(div);
	},
	SetupDialogForm: function (id) {
		if ($('#' + id).length === 0) {
			$(document.body).append('<div id=' + id + '></div>');
		} else {
			try {
				$('#' + id).dialog("destroy");
			} catch (e) {
				//console.log(e);
			}
		}
	},

	init: function () {
		ns.login.DoLogin();
		ns.status.check();
	},

	/*
	* array of functions used in the webmail WYSIWYG editor
	*/
	wmedit: {
		get_iframe: function () {
			return document.getElementById('contentiframe').contentWindow;
		},
		init: function () {
			document.wmcompose.ctype.value = 'plain';
			document.getElementById('contentiframe').contentWindow.document.designMode = 'on';
		},
		//	copy_submit: function () {
		//		if (EditorMode=='HTML') {
		//			document.wmcompose.msgbody.value=wmedit.get_iframe().document.body.innerHTML;
		//		}
		//	},
		wmformat: function (what) {
			ns.wmedit.get_iframe().document.execCommand(what, false, null);
			ns.wmedit.get_iframe().focus();
		},
		html_action: function (what) {
			ns.wmedit.get_iframe().document.execCommand(what);
		},
		doImage: function () {
			var imgSrc = prompt('Enter Image URL', '');
			if (imgSrc !== null) ns.wmedit.get_iframe().document.execCommand('InsertImage', false, imgSrc);
		},
		doLink: function () {
			ns.wmedit.get_iframe().document.execCommand('CreateLink', false, null);
		},
		SetForeColour: function () {
			var fCol = prompt('Enter Font Colour', '');
			if (fCol !== null) ns.wmedit.get_iframe().document.execCommand('ForeColor', false, fCol);
		},
		SetFont: function (what, how) {
			ns.wmedit.get_iframe().document.execCommand(what, false, how);
		},
		selOn: function (ctrl) {
			ctrl.style.borderColor = '#000000';
			ctrl.style.cursor = 'hand';
		},
		selOff: function (ctrl) {
			ctrl.style.borderColor = '#F0F0F0';
		}
	},

	openNav: function () {
		document.getElementById("sidenav").style.width = "250px";
		//document.body.style.backgroundColor = "rgba(0,0,0,0.4)";
	},

	closeNav: function () {
		document.getElementById("sidenav").style.width = "0";
		//document.body.style.backgroundColor = "white";
	},
	/*
		function removejscssfile(filename, filetype){
			var targetelement=(filetype=="js")? "script" : (filetype=="css")? "link" : "none" //determine element type to create nodelist from
			var targetattr=(filetype=="js")? "src" : (filetype=="css")? "href" : "none" //determine corresponding attribute to test for
			var allsuspects=document.getElementsByTagName(targetelement)
			for (var i=allsuspects.length; i>=0; i--){ //search backwards within nodelist for matching elements to remove
			if (allsuspects[i] && allsuspects[i].getAttribute(targetattr)!=null && allsuspects[i].getAttribute(targetattr).indexOf(filename)!=-1)
				allsuspects[i].parentNode.removeChild(allsuspects[i]) //remove element by calling parentNode.removeChild()
			}
			}

			removejscssfile("somescript.js", "js") //remove all occurences of "somescript.js" on page
			removejscssfile("somestyle.css", "css") //remove all occurences "somestyle.css" on page
	*/
	loadtheme: function (theme) {
		//<link rel="stylesheet" type="text/css" href="/lib/themes/default/style.css">
		var filename = "/lib/themes/" + theme + "/style.css";
		var allsuspects = document.getElementsByTagName("link");
		var loaded = false;
		for (var i = allsuspects.length; i >= 0; i--) {
			if (!allsuspects[i]) continue;
			var href = allsuspects[i].getAttribute("href");
			if (href == null) continue;
			if (href.indexOf("style.css") == -1) continue;
			if (href != filename) {
				//				if (allsuspects[i].getAttribute("href").indexOf("style.css")!=-1) {
				//					console.log("allsuspects[i].getAttribute(\"href\")",allsuspects[i].getAttribute("href"));
				allsuspects[i].parentNode.removeChild(allsuspects[i]);
				//				}
			} else {
				loaded = true;
			}
		}
		if (!loaded) {
			var fileref = document.createElement("link");
			fileref.setAttribute("rel", "stylesheet");
			fileref.setAttribute("type", "text/css");
			fileref.setAttribute("href", filename);
			if (typeof fileref != "undefined") {
				document.getElementsByTagName("head")[0].appendChild(fileref);
			}
		}
	}
}
