<?php

require_once "./spell_parser.inc";
header("Content-type: text/html; charset: utf-8");


?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <style type="text/css">
		.HA-spellcheck-error { 
			border-bottom: 2px dotted #FF0000; 
			cursor: pointer; 
		}

		.HA-spellcheck-same {
			color: #000000;
			font-weight: bold;
		}

		.HA-spellcheck-hover { 
			color: #000000; 
		}

		.HA-spellcheck-fixed { 
			border-bottom: 1px dotted #0b8; 
		}

		.HA-spellcheck-current {
			color: #000000;
			background-color: #EBEBEB;
			font-weight: bold;
		}

		.HA-spellcheck-suggestions { 
			display: none; 
		}

		#HA-spellcheck-dictionaries { 
			display: none; 
		}

		a:link, a:visited {
			color: #55e; 
		}

		body {
			font-family: tahoma,verdana,sans-serif; 
			font-size: 14px; 
			color: #666666;
		}

	</style>
  </head>
  <body onload="window.parent.finishedSpellChecking();">
<?php



$GLOBALS['spellerId'] = 0;
$GLOBALS['dict'] = 'en';

if (isset($_REQUEST['dictionary'])) {
	if ($_REQUEST['dictionary'] != "") {
		$GLOBALS['dict'] = $_REQUEST['dictionary'];
	} 	
}

if ($_REQUEST['init'] == 1) {
	// don't put spaces between these as js is going to tokenize them up
	echo "<div id='HA-spellcheck-dictionaries'>en,en_GB,en_US,en_CA,sv_SE,de_DE,pt_PT</div>";
}

$spell_parser = new Spell_Parser();
$spell_parser->setLanguage($GLOBALS['dict']);

if (get_magic_quotes_gpc()) {
	foreach ($_REQUEST as $k => $v) {
		$_REQUEST["$k"] = stripslashes($v);
	}
}
$spell_parser->parseString('<spellThis>'.utf8_decode($_REQUEST['content']).'</spellThis>', true);


?>
  <body>
</html>