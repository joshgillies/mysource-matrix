<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: export_to_xml.php,v 1.9 2009/02/13 00:20:12 bpearson Exp $
*
*/

/**
* Creates XML based on an asset ID provided.
*
* @author  Edison Wang <ewang@squiz.net>
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix
*/



/*
 *
 *
 * Example usage:
 * php scripts/import/export_to_xml.php . 3:35,4:36 1 >export.xml
 * 
 * First argument specifies system root path
 *
 * Second argument specifies which asset should be moved underneath which parent asset, 
 * 3:35 means asset with id 3 will be moved underneath parent asset with id 35
 *
 * Third argument specifies create link type
 *
 */

error_reporting(E_ALL);
ini_set('memory_limit', '1024M');
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$asset_infos = (isset($_SERVER['argv'][2])) ? explode(',',$_SERVER['argv'][2]) : Array();
if (empty($asset_infos)) {
	trigger_error("You need to supply the asset id for the asset you want to export and parent asset it will link to as the second argument with format 3:75,4:46 (assetid 3 links to assetid 75, assetid 4 links to asset id 46)\n", E_USER_ERROR);
}



$initial_link_type = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : '';
if (empty($initial_link_type)) {
	trigger_error("You need to supply the initial link type as the third argument\n", E_USER_ERROR);
}


require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

// log in as root
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$asset_id_map = Array();

echo "<actions>\n";

foreach($asset_infos as $asset_info) {
	$asset_from_to_id = explode(':',$asset_info);
	if(!isset($asset_from_to_id[0]) || !isset($asset_from_to_id[1])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
	printCreateXML($asset_from_to_id[0], $asset_from_to_id[1], $initial_link_type);
}
foreach($asset_infos as $asset_info) {
	$asset_from_to_id = explode(':',$asset_info);
	if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
	printAttributeXML($asset_from_to_id[0]);
}
foreach($asset_infos as $asset_info) {
	$asset_from_to_id = explode(':',$asset_info);
	if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
	printNoticeLinksXML($asset_from_to_id[0]);
}
foreach($asset_infos as $asset_info) {
	$asset_from_to_id = explode(':',$asset_info);
	if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
	printPermissionXML($asset_from_to_id[0]);
}

echo "</actions>\n\n";


	/**
	* Lovely recursing function to create the XML
	*
	* @param string	$type_code	the code name for the asset type that you want to refresh
	*
	* @return void
	* @access public
	*/
	function printCreateXML($asset_id, $parent, $link_type, $value='', $is_dependant=0, $is_exclusive=0) {

		global $asset_id_map;

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		// system assets are not allowed to be exported.
		if (replace_system_assetid($asset->id) != NULL)	trigger_error("Can not export system asset!\n", E_USER_ERROR);


		$action_code = getAssetType($asset).'_'.$asset->id;

		echo_headline('CREATING ASSET: '.$asset->name);
		$asset_id_map[$asset->id] = $action_code;

		echo "<action>\n";
		echo "   <action_id>create_".$action_code."</action_id>\n";

		if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant(getAssetType($asset), 'file')) {
			$file_path = _saveFileAsset($asset);
			echo "   <action_type>create_file_asset</action_type>\n";
			echo "   <file_path>".$file_path."</file_path>\n";
		} else {
			echo "   <action_type>create_asset</action_type>\n";
		}

		echo "   <type_code>".getAssetType($asset)."</type_code>\n";
		echo "   <link_type>".$link_type."</link_type>\n";
		echo "   <parentid>".$parent."</parentid>\n";
		echo "   <value>".$value."</value>\n";
		echo "   <is_dependant>".$is_dependant."</is_dependant>\n";
		echo "   <is_exclusive>".$is_exclusive."</is_exclusive>\n";
		echo "</action>\n\n";


		// then, if it has web paths, we add those
		$paths = $asset->getWebPaths();
		foreach ($paths as $path) {

			echo "<action>\n";
			echo "   <action_id>add_".$action_code."_path</action_id>\n";
			echo "   <action_type>add_web_path</action_type>\n";
			echo "   <asset>[[output://create_".$action_code.".assetid]]</asset>\n";
			echo "   <path>".$path."</path>\n";
			echo "</action>\n\n";
		}


		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				$parent = '[[output://create_'.$action_code.'.assetid]]';

				// If the asset already exists, let's link to it; otherwise, create it.
				if (array_key_exists($link_info['minorid'], $asset_id_map)) {
					printLinkXML($parent, $link_info, $action_code);
				} else {
					printCreateXML($link_info['minorid'], $parent, $link_info['link_type'], $link_info['value'], $link_info['is_dependant'], $link_info['is_exclusive']);
				}

			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				$parent = '[[output://create_'.$action_code.'.assetid]]';

				if (array_key_exists($link_info['minorid'], $asset_id_map)) {
					printLinkXML($parent, $link_info, $action_code);
				} else {
					printCreateXML($link_info['minorid'], $parent, $link_info['link_type'], $link_info['value'], $link_info['is_dependant'], $link_info['is_exclusive']);
				}

			}
		}


		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printXML

	function printLinkXML($parent, $link_info, $action_code) {

		global $asset_id_map;
		
		// if we created the asset, get it from array, otherwise, if it's a system asset, we have to use system asset name.
		if (isset($asset_id_map[$link_info['minorid']])){
			$remap_id =	"[[output://create_".$asset_id_map[$link_info['minorid']].".assetid]]"; 
		}
		else {
			//before we use asset id, we should make sure if it's a system asset, we use system asset name instead of id. 
			//because new system may have different system assets ids.
			$system_asset_name = replace_system_assetid($link_info['minorid']);
			$system_asset_name == NULL ? $remap_id = $link_info['minorid'] : $remap_id = "[[system://".$system_asset_name."]]" ;
		}

		echo "<action>\n";
		echo "   <action_id>link_".$action_code."</action_id>\n";
		echo "   <action_type>create_link</action_type>\n";
		echo "   <asset>".$remap_id."</asset>\n";
		echo "   <assetid>".$parent."</assetid>\n";
		echo "   <is_major>0</is_major>\n";
		echo "   <link_type>".$link_info['link_type']."</link_type>\n";
		echo "   <value>".$link_info['value']."</value>\n";
		echo "   <is_dependant>".$link_info['is_dependant']."</is_dependant>\n";
		echo "   <is_exclusive>".$link_info['is_exclusive']."</is_exclusive>\n";
		echo "</action>\n\n";

	}

	function printAttributeXML($asset_id) {

		global $asset_id_map;

		$assets_done = Array();

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		// print some attributes
		foreach ($asset->vars as $attr_name => $attr_info) {
			$attr = $asset->getAttribute($attr_name);
			$value = $attr->getContent();
			// unserilize it, we want clean script
			if(isSerialized($value) && !empty($value)){
				$value = var_export(unserialize($value),TRUE);
			}
			// if the value is an assetid attribute, we should match it with asset_id_map first
			if (preg_match("/assetid/",$attr_name) && isset($asset_id_map[$value])){
				$value =	"[[output://create_".$asset_id_map[$value].".assetid]]"; 
			}
			$assets_done[$asset->id] = TRUE;

			if (!empty($value)) {
				echo "<action>\n";
				echo "   <action_id>set_".$asset_id_map[$asset_id].'_'.$attr_name."</action_id>\n";
				echo "   <action_type>set_attribute_value</action_type>\n";
				echo "   <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "   <attribute>".$attr_name."</attribute>\n";
				if ($attr_name == 'html') { $value = _parseValue($value); }
				echo "   <value><![CDATA[".$value."]]></value>\n";
				echo "</action>\n\n";
			}
		}

		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printAttributeXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printAttributeXML($link_info['minorid']);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printAttributeXML()

	function printNoticeLinksXML($asset_id) {

		global $asset_id_map;

		$assets_done = Array();

		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_LINK_NOTICE, '', FALSE, 'major');

		foreach ($notice_links as $notice_link) {
			// if we created the asset, get it from array, otherwise, if it's a system asset, we have to use system asset name.
			if (isset($asset_id_map[$asset_id])){
				$remap_id =	"[[output://create_".$asset_id_map[$asset_id].".assetid]]" ;
			}
			else {
				//before we use asset id, we should make sure if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = replace_system_assetid($asset_id);
				$system_asset_name == NULL ? $remap_id = $asset_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}
			printLinkXML($remap_id, $notice_link, 'notice_'.$asset_id.'_to_'.$notice_link['minorid']);

		}//end foreach

		$assets_done[$asset_id] = TRUE;

		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printNoticeLinksXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printNoticeLinksXML($link_info['minorid']);
				}
			}
		}


		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printNoticeLinksXML()

	function printPermissionXML($asset_id) {

		global $asset_id_map;

		$assets_done = Array();

		// Now, some permissions
		$read_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_READ, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($read_permissions as $permission_id => $permission_granted) {
			// if the asset is created by this script, use asset_id_map	
			if (isset($asset_id_map[$permission_id])){
				$remap_id =	"[[output://create_".$asset_id_map[$permission_id].".assetid]]" ;
			}
			else {
				//if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}
			
		

			// if the action is to deny a previous permission, make sure we will remove the permission first
			// even removing it will possibly fail, it will not affect the result.
			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_read_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "    <permission>1</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_read_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
			echo "    <permission>1</permission>\n";
			echo "    <granted>".$permission_granted."</granted>\n";
			echo "    <userid>".$remap_id."</userid>\n";
			echo "</action>\n";

		}//end foreach

		$write_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_WRITE, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($write_permissions as $permission_id => $permission_granted) {

			// if the asset is created by this script, use asset_id_map	
			if (isset($asset_id_map[$permission_id])){
				$remap_id =	"[[output://create_".$asset_id_map[$permission_id].".assetid]]" ;
			}
			else {
				//if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}

			// if the action is to deny a previous permission, make sure we will remove the permission first
			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_write_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "    <permission>2</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_write_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
			echo "    <permission>2</permission>\n";
			echo "    <granted>".$permission_granted."</granted>\n";
			echo "    <userid>".$remap_id."</userid>\n";
			echo "</action>\n";

		}//end foreach

		$admin_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_ADMIN, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($admin_permissions as $permission_id => $permission_granted) {

			if (isset($asset_id_map[$permission_id])){
				$remap_id =	"[[output://create_".$asset_id_map[$permission_id].".assetid]]" ;
			}
			else {
				$system_asset_name = replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}

			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_admin_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "    <permission>3</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_admin_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
			echo "    <permission>3</permission>\n";
			echo "    <granted>".$permission_granted."</granted>\n";
			echo "    <userid>".$remap_id."</userid>\n";
			echo "</action>\n";

		}//end foreach

		$assets_done[$asset_id] = TRUE;

		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printPermissionXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printPermissionXML($link_info['minorid']);
				}
			}
		}


	}//end printPermissionXML()

	function _saveFileAsset(&$asset) {

		$file_info = $asset->getExistingFile();
		$file_type = getAssetType($asset);
		$export_path = 'export/'.$file_type.'/'.$file_info['filename'];

		// check to see if an export/ directory exists. If not, create it.
		if (!file_exists('export/')) {
			mkdir("export", 0775);
		}

		// echeck to see if the type_code directory exists
		if (!file_exists('export/'.$file_type.'/')) {
			mkdir('export/'.$file_type, 0775);
		}

		if (copy($file_info['path'], $export_path)) {
			return $export_path;
		} else {
			die('Could not copy file');
		}


	}//end _saveFileAsset()

	function _parseValue($value) {
		global $asset_id_map;

		$shadow_reg = '|.\/?a=(\d+:\w*)|';
		$normal_reg = '|.\/?a=(\d+)|';
		$shadow_matches = Array();
		$normal_matches = Array();

        preg_match_all($shadow_reg, $value, $shadow_matche);
		preg_match_all($normal_reg, $value, $normal_matches);

		if(isset($shadow_matches[1]))		$shadow_matches = $shadow_matches[1];
		if(isset($normal_matches[1])) 	$normal_matches = $normal_matches[1];
		$replace_assetids = Array();
		foreach ($shadow_matches as $data) {
				$replace_assetids[] = $data;
		}
		foreach ($normal_matches as $data) {
				$replace_assetids[] = $data;
		}
		$replace_assetids = array_unique($replace_assetids);

		foreach ($replace_assetids as $replace_assetid) {
			$value = preg_replace('|.\/?a='.$replace_assetid.'|', '?a=[[output://create_'.$asset_id_map[$replace_assetid].'.assetid]]', $value);
		}

		return $value;

	}


	/**
	* Returns the asset type that this management class is working for
	* Borrowed from asset_management.inc, but modified for our purposes
	*
	* @return string
	* @access public
	*/
	function getAssetType(&$asset)
	{
		$class = get_class($asset);
		return $class;

	}//end getAssetType()

	/**
	* Print a headline to STDERR
	*
	* @param string		$s	the headline
	*
	* @return void
	* @access public
	*/
	function echo_headline($s)
	{
		fwrite(STDERR, "--------------------------------------\n$s\n--------------------------------------\n");

	}//end echo_headline()


	/**
	* Print a headline to STDERR
	*
	* @param string		$s	the headline
	*
	* @return void
	* @access public
	*/
	function echo_text($s)
	{
		fwrite(STDERR, "$s\n");

	}//end echo_headline()

	/**
	* Replace a current system asset's id with its name, because new system might have different asset id. a name will be safer
	*
	* @param string		$s assetid
	*
	* @return void|String
	* @access public
	*/
	function replace_system_assetid($assetid){
		foreach($GLOBALS['SQ_SYSTEM']->am->_system_assetids as $asset_name => $asset_id){
			if($assetid == $asset_id) {
				return $asset_name;
			}
		}
		return NULL;
	}
	
	
	/**
	* 
	* Test if a string is serialized
	* @param string	
	*
	* @return boolean
	* @access public
	*/
	function isSerialized($str) {
   	 	return ($str == serialize(false) || @unserialize($str) !== false);
	}

?>


