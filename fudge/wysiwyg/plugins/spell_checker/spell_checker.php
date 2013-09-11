<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: spell_checker.php,v 1.18 2013/09/11 03:30:49 ewang Exp $
*
*/

/**
* Spell Checker Popup for the WYSIWYG
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix
*/
require_once dirname(__FILE__).'/../../../../core/include/init.inc';
include_once dirname(__FILE__).'/../../../../data/private/conf/tools.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}
if(get_unique_token() !== $_POST['token']) exit();

header('Content-type: text/html; charset: utf-8');

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
		$GLOBALS['dict'] = SQ_TOOL_SPELL_CHECKER_LANG;

		// the user has asked to change the dictionary and re-check
		// dictionary is now a global preference
		/*
		if (isset($_REQUEST['dictionary'])) {
			if ($_REQUEST['dictionary'] != "") {
				$GLOBALS['dict'] = $_REQUEST['dictionary'];
			}
		}
		*/

		/*
		// leave out the dicrtionary support for the moment
		if ($_REQUEST['init'] == 1) {
			// don't put spaces between these as js is going to tokenize them up
			echo "<div id='HA-spellcheck-dictionaries'>en,en_GB,en_US,en_CA,sv_SE,de_DE,pt_PT</div>";
		}
		*/

		if (get_magic_quotes_gpc()) {
			foreach ($_REQUEST as $k => $v) {
				$_REQUEST["$k"] = stripslashes($v);
			}
		}

		require_once DIRNAME(__FILE__).'/spell_parser.inc';
		require_once 'XML/XML_HTMLSax.php';

		$handler = new Spell_Parser();
		$handler->setLanguage($GLOBALS['dict']);

		$parser = new XML_HTMLSax();
		$parser->set_object($handler);
		$parser->set_element_handler('openHandler', 'closeHandler');
		$parser->set_data_handler('dataHandler');

		$string_to_parse = stripslashes($_POST['content']);

		$parser->parse($string_to_parse);

		?>
	<body>
</html>
