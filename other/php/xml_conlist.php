<html>
<head><title>xmlrpc contact list</title></head>
<body>
<?php
require("_xmlrpc.php");
require("_global.php");

$f=new xmlrpcmsg('contacts.list', array(new xmlrpcval($sys_username, "string"), new xmlrpcval($sys_password, "string")));
$c=new xmlrpc_client("/xml-rpc/", $sys_host, $sys_port);
$c->setDebug(0);
$r=$c->send($f);
if (!$r) { die("send failed"); }
if ($r->faultCode()) {
	print "Fault: ";
	print "Code: ". $r->faultCode(). " Reason '". $r->faultString(). "'<BR>";
} else {
	$v=xmlrpc_decode($r->value());
	$max=sizeof($v); 
	print "found $max entries\n";
	print "<TABLE BORDER=1>";
	print "<TR><TD NOWRAP>Contact ID</TD><TD NOWRAP>User Name</TD><TD NOWRAP>Surname</TD><TD NOWRAP>Given Name</TD><TD NOWRAP>Organization</TD><TD NOWRAP>Work Number</TD><TD NOWRAP>E-Mail</TD></TR>";
	for ($i=0;$i<$max;$i++) {
		print "<TR>\n";
		print "<TD NOWRAP><A HREF=xml_conread.php?contactid=".$v[$i]["contactid"].">".$v[$i]["contactid"]."</A>&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["username"].     "&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["surname"].      "&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["givenname"].    "&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["organization"]. "&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["worknumber"].   "&nbsp;</TD>\n";
		print "<TD NOWRAP>". $v[$i]["email"].        "&nbsp;</TD>\n";
		print "</TR>\n";
	}
	print "</TABLE>";
}
?>
</body>
</html>
