<?php
/**
* Script for running the WYSIWYG Replace Text Plugin on all editable content. 
* You must specify the root nodes (comma seperated) to be looked under.
* You may optionally specify the root nodes to exclude (comma seperated).
* There are three configuration options you may change in the file. See below.
*
* 
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/

// START Configuration Options:

// 1- Asset types to allow. Add/Remove types from array. Empty for all types allowed.
$type_code = Array (
		'page',
	);

// 2- Whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes (True or False)
$strict_type_code = FALSE;

// 3- Enable/Disable the following options (0 or 1).
$options = Array (
		//non-extreme options
		'Remove <font> tags' => 1,
		'Remove double spaces' => 1,
		'Remove non-HTML tags' => 1,
		'Change Microsoft Words bullets' => 1,
		'Remove soft hyphens' => 1,
		//extreme options
		'Remove style attribute' => 0,
		'Remove class attribute' => 0,
		'Remove <table> tags' => 0,
		'Remove <span> tags' => 0,
		'Remove all empty tags' => 0,
		'Remove all tags attributes (except HREF and SRC)' => 0,
	);

// END Configuration Options:

	error_reporting(E_ALL);
	if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

	$args = count($_SERVER['argv']);
	if ($args > 4 || $args < 3) {
		echo "This script needs to be run in the following format:\n\n";
		echo "\tphp replace_text.php SYSTEM_ROOT root_node_ids [exclude_root_node_ids]\n\n";
		echo "\tEg. php scripts/replace_text.php . 10,5 7,2\n\n";
		echo "Also note there are 3 configurable options in the script: By default non-extreme options are run on all 'page' types\n\n";
		exit(1);
	}

	$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
	if (empty($SYSTEM_ROOT)) {
		echo "ERROR: You need to supply the path to the System Root as the first argument\n";
		exit();
	}

	if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
		echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
		exit();
	}

	if (isset($_SERVER['argv'][2])) {
		$root_nodes = explode(',', $_SERVER['argv'][2]);
	}

	$excl_nodes = Array();
	if (isset($_SERVER['argv'][3])) {
		$excl_nodes = explode(',', $_SERVER['argv'][3]);
	}

	require_once $SYSTEM_ROOT.'/core/include/init.inc';

	$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	
	//find all asset id's to be excluded
	$excl_ids = Array();
	foreach ($excl_nodes as $node) {
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($node, $type_code, $strict_type_code);
		foreach ($children as $child_id => $info) {
			$excl_ids[] = $child_id;
		}
	}
	
	foreach ($root_nodes as $node) {
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($node, $type_code, $strict_type_code, FALSE);
		foreach ($children as $child_id => $info) {
			if (!in_array($child_id, $excl_nodes) && !in_array($child_id, $excl_ids)) {
				$contents = $GLOBALS['SQ_SYSTEM']->am->getEditableContents($child_id);
				if ($contents) {
					foreach ($contents as $id => $edit) {
						echo "Examining wysiwyg content type of Asset ID: $id\n";
						$edited = process_replace_text($edit, $options);
						if ($edited !== FALSE) {
							$GLOBALS['SQ_SYSTEM']->am->setEditableContents($id, $edited);
						} else {
							die ("There is a crazy error in this script. Most likey the options array has been misconfigured\n");
						}
					}
				} else {
					if (isset ($info[0]['type_code'])) {
						$type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($info[0]['type_code'], Array('name', 'type'));
					} else {
						continue;
					}
					foreach ($type_info as $name => $type) {
						if ($type['type'] == 'wysiwyg'){
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
							$contents = $asset->attr($name);
							echo "Examining wysiwyg contents of attribute '$name' of Asset ID: $child_id\n";
							$edited = process_replace_text($contents, $options);
							if ($edited === FALSE) die ("There is a crazy error in this script. Most likey the options array has been misconfigured\n");
							$asset->setAttrValue($name, $edited);
							$asset->saveAttributes();
						}
						
					}
				}
			}
		}
	}

	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

	/**
	* Process the replacing of text given the html
	* 
	* @param Array	$options The configured options (enable/disable) extreme/non-extreme options.
	* @param String	$html    The html text we need to process
	*
	* @return String
	* @access public
	*/
	function process_replace_text($html, $options)
	{
		$reg = null;
		$rep = null;	
		$curHTML = $html;
		$HTMLtags = "!--|a|abbr|above|acronym|address|applet|array|area|b|base|basefont|bdo|bgsound|big|blink|blockquote|body|box|br|blink|button|caption|center|cite|code|col|colgroup|comment|dd|del|dfn|dir|div|dl|dt|em|embed|fieldset|fig|font|form|frame|frameset|h1|h2|h3|h4|h5|h6|head|hr|html|i|id|iframe|ilayer|img|input|ins|isindex|kbd|label|layer|legend|li|link|listing|map|marquee|menu|meta|multicol|nextid|nobr|noframes|nolayer|note|noscript|object|ol|option|keygen|optgroup|p|param|pre|q|quote|range|root|s|samp|script|select|small|sound|spacer|span|sqrt|strike|strong|style|sub|sup|table|tbody|td|text|textarea|tfoot|th|thead|title|tr|tt|u|ul|var|wbr|xmp";
		$bullet = urldecode("%B7");
		$shy    = urldecode("%AD");
		foreach ($options as $key => $value) {
			if ($value == 1) {
				// Bug #3204  - Remove Word Document HTML Clipboard Tags, that get pasted through on Firefox 3
				$localreg = "%<link rel=\"[^\"]*\" href=\"file[^\"]*\">%i";
				$localrep = "";
				$curHTML = preg_replace($localreg, $localrep, $curHTML);
				$wordreg = "%<w\:[^>]*>(.*?)<\/w\:[^>]*>%i";
				$wordrep = "";
				$curHTML = preg_replace($wordreg, $wordrep, $curHTML);
				switch ($key) {
					case 'Remove <font> tags':
						$reg = "%<\/?font ?[^>]*>%i";
						$rep = "";
						break;
					case 'Remove double spaces':
						//#3827 -- using regexp literal 
						$reg = "%(\s|&nbsp;){2,}%i";
						$rep = "$1";
						break;
					case 'Remove non-HTML tags':
						$reg = "%<(?!(\/?(".$HTMLtags.")[> ]))([^>]*)>%i";
						$rep = "";
						break;
					case 'Change Microsoft Words bullets':
						$reg = "%<p[^>]*>(".$bullet."|&middot;)(.*?)<\/p>%i";
						$rep = "<li>$2";
						break;
					case 'Remove soft hyphens':
						$reg = "%(&shy;?|".$shy.")%i";
						$rep = "";
						break;
					case 'Remove style attribute':
						$reg = "% style=\"?[^\">]*\"?%i";
						$rep = "";
						break;
					case 'Remove class attribute':
						$reg = "% class=\"?[^\">]*[\"]?%i";
						$rep = "";
						break;
					case 'Remove <table> tags':
						$reg = "%<(table|/table|tr|tbody|/tbody|td|th) ?[^>]*>%i";
						$rep = "";
						$curHTML = preg_replace($reg, $rep, $curHTML);
						$reg = "%<(/tr|/td|/th)>%i";
						$rep = "<br />";
						break;
					case 'Remove <span> tags':
						$reg = "%<\/?span( [^>]*>|>)%i";
						$rep = "";
						break;
					case 'Remove all empty tags':
						$reg = "%<([A-Z][A-Z0-9]*)( [^>]*)?>(&nbsp;| |\n|\t)*<\/\\1>%i";
						$rep = "";
						break;
					case 'Remove all tags attributes (except HREF and SRC)':
						$reg = '%<([^/ >]+)[^>]*?([^>]*?( (src|href)="?[^>"]*"?)[^>]*?)*[^>]*?>%i';
						$rep = "<$1$3>";
						break;
					default	: return false;
				}
				// BUG#928 - special condition to allow empty anchor tag
				if ($key == 'Remove all empty tags') {
					$reg2 = "%(<A NAME[^>]*?>)(&nbsp;| |\n|\t)*(</A>)%i";
					$rep2 = "$1matrix_anchor_tmp$3";
					$curHTML = preg_replace($reg2, $rep2, $curHTML);
					$reg2 = "%(<A ID[^>]*?>)(&nbsp;| |\n|\t)*(</A>)%i";
					$rep2 = "$1matrix_anchor_tmp$3";
					$curHTML = preg_replace($reg2, $rep2, $curHTML);
				}
				$curHTML = preg_replace($reg, $rep, $curHTML); 
				
				if ($key == 'Remove all empty tags') {
					$reg3 = "%(<A NAME[^>]*?>)matrix_anchor_tmp(</A>)%i";
					$rep3 = "$1$2";
					$curHTML = preg_replace($reg3, $rep3, $curHTML);
					$reg3 = "%(<A ID[^>]*?>)matrix_anchor_tmp(</A>)%i";
					$rep3 = "$1$2";
					$curHTML = preg_replace($reg3, $rep3, $curHTML);
				}
			}
		}
		return $curHTML;
		
	}//end process_replace_text()

?>
