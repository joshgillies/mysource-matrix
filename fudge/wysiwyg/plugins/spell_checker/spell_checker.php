<?php

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
	<body onload="window.parent.finishedSpellChecking();" bgcolor="#FFFFFF">
		<?php
		$GLOBALS['spellerId'] = 0;
		$GLOBALS['dict'] = 'en';

		// the user has asked to change the dictionary and re-check
		if (isset($_REQUEST['dictionary'])) {
			if ($_REQUEST['dictionary'] != "") {
				$GLOBALS['dict'] = $_REQUEST['dictionary'];
			}
		}

		if ($_REQUEST['init'] == 1) {
			// don't put spaces between these as js is going to tokenize them up
			echo "<div id='HA-spellcheck-dictionaries'>en,en_GB,en_US,en_CA,sv_SE,de_DE,pt_PT</div>";
		}

		if (get_magic_quotes_gpc()) {
			foreach ($_REQUEST as $k => $v) {
				$_REQUEST["$k"] = stripslashes($v);
			}
		}

		require_once DIRNAME(__FILE__)."/spell_parser.inc";
		require_once "XML/XML_HTMLSax.php";

		$handler = new Spell_Parser();
		$handler->setLanguage($GLOBALS['dict']);

		$parser =& new XML_HTMLSax();
		$parser->set_object($handler);
		$parser->set_option('XML_OPTION_TRIM_DATA_NODES');
		$parser->set_element_handler('openHandler', 'closeHandler');
		$parser->set_data_handler('dataHandler');

		$string_to_parse = stripslashes(utf8_decode($_POST['content']));

		// replace HTML entities (&nbp; etc) with their equivalents (' ' etc)
		$trans_tbl = get_html_translation_table(HTML_SPECIALCHARS);
		$trans_tbl = array_flip($trans_tbl);
		// special case for &nbsp; because the charter returned is not recognised and ends up
		// being displayed as a question mark (?)
		$trans_tbl['&nbsp;'] = ' ';
		$string_to_parse = strtr($string_to_parse, $trans_tbl);

		$parser->parse($string_to_parse);
		
		?>
	<body>
</html>