<html>
<head><title>xmlrpc contact read</title></head>
<body>
<?php
require("_xmlrpc.php");
require("_global.php");

if (array_key_exists('contactid', $HTTP_POST_VARS)) {
	$contactid=@$HTTP_POST_VARS["contactid"];
} else if (array_key_exists('contactid', $HTTP_GET_VARS)) {
	$contactid=@$HTTP_GET_VARS["contactid"];
} else {
	$contactid=0;
}
print "<FORM ACTION=\"xml_conread.php\" METHOD=\"POST\" NAME=\"contact\">\n";
print "enter a contact number to retrieve its record<BR>\n";
print "<INPUT NAME=\"contactid\" VALUE=\"${contactid}\">\n";
print "<input type=\"submit\" value=\"go\" name=\"submit\"></FORM>\n";
if ($contactid!=0) {
	$f=new xmlrpcmsg('contacts.read', array(new xmlrpcval($sys_username, "string"), new xmlrpcval($sys_password, "string"), new xmlrpcval($sys_domain, "string"), new xmlrpcval($contactid, "int")));
	$c=new xmlrpc_client("/xml-rpc/", $sys_host, $sys_port);
	$c->setDebug(0);
	$r=$c->send($f);
	if (!$r) { die("send failed"); }
	if ($r->faultCode()) {
		print "Fault: ";
		print "Code: ". $r->faultCode(). " Reason '". $r->faultString(). "'<BR>";
	} else {
		$v=xmlrpc_decode($r->value());
		print "<TABLE BORDER=1>";
		print "<TR><TD>Contact ID  </TD><TD>". $v["contactid"].    "&nbsp;</TD></TR>\n";
		print "<TR><TD>Username    </TD><TD>". $v["username"].     "&nbsp;</TD></TR>\n";
		print "<TR><TD>Surname     </TD><TD>". $v["surname"].      "&nbsp;</TD></TR>\n";
		print "<TR><TD>Given name  </TD><TD>". $v["givenname"].    "&nbsp;</TD></TR>\n";
		print "<TR><TD>Company Name</TD><TD>". $v["organization"]. "&nbsp;</TD></TR>\n";
		print "<TR><TD>Worknumber  </TD><TD>". $v["worknumber"].   "&nbsp;</TD></TR>\n";
		print "<TR><TD>E-Mail      </TD><TD>". $v["email"].        "&nbsp;</TD></TR>\n";
		print "</TABLE>";
	}
}
?>
</body>
</html>
