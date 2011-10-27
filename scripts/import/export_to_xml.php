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
* $Id: export_to_xml.php,v 1.20 2011/10/27 02:59:18 akarelia Exp $
*
*/

/**
* Creates XML based on an asset ID provided.
*
* @author  Edison Wang <ewang@squiz.net>
* @author  Avi Miller <amiller@squiz.net>
* @version $Revision: 1.20 $
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
$warned = FALSE;
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
	printMetadataXML($asset_from_to_id[0]);
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

		$dependants = Array();
		if (getAssetType($asset) != 'Design_Customisation' && getAssetType($asset) != 'Design' && getAssetType($asset) != 'Design_Css') {
			// Then, if it has dependant children, we add those too
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design' || getAssetType($asset) == 'Design_Css') {
			
			// okie this part is a bit tricky
			// we need to check if 
			//	- we are dealing with design
			//	- if design has a parse file attached
			//	- and the attribute name we are dealing is 'contents'
			//	- last but not least the value isnt not empty
			$parse_file = $asset->data_path.'/parse.txt';
			if (is_file($parse_file)) _updateParseFileForDesign($asset_id_map[$asset_id], $parse_file);

			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', FALSE, 'major', NULL, TRUE);

		} else if (getAssetType($asset) == 'Design_Customisation') {
			// if we are dealing with customisations, then only deal with
			// the design areas that are customised rest of the design areas
			// will be generated once Matrix processes the parse file
			$dependants = $asset->getCustomisedAreas();
			// now this customistation can further have customisations
			// take care of those ones too in our export
			$dependants = array_merge($dependants, $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, 'design_customisation'));
		}

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

			if (!empty($value) && !((getAssetType($asset) == 'Design' || getAssetType($asset) == 'Design_Css') && $attr_name == 'contents')) {
				// if default character set is utf-8 we dont wanna mess around the characters
				// we are for now just checking for it not to be utf-8 but can append other encoding
				// we might want to skip later on
				if (strtolower(SQ_CONF_DEFAULT_CHARACTER_SET) != 'utf-8') {
					$result = '';
					// escape everything else (chars > 126)
					for ($i = 0; $i < strlen($value); ++$i) {
						$ord = ord($value[$i]);
						if ($ord > 126) {
							$result .= '&#'.$ord.';';
						} else {
							$result .= $value[$i];
						}
					}// end for

					if ($result !== '') $value = $result;
					$value = preg_replace('/(\r\n)+/', '<br/>' ,$value);
				}

				echo "<action>\n";
				echo "   <action_id>set_".$asset_id_map[$asset_id].'_'.$attr_name."</action_id>\n";
				echo "   <action_type>set_attribute_value</action_type>\n";
				echo "   <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "   <attribute>".$attr_name."</attribute>\n";
				$value = _parseValue($value, $attr_name, 'set_'.$asset_id_map[$asset_id].'_'.$attr_name);

				echo "   <value><![CDATA["._escapeCDATA($value)."]]></value>\n";
				echo "</action>\n\n";
			}
		}

		$dependants = Array();
		if (getAssetType($asset) != 'Design_Customisation' && getAssetType($asset) != 'Design' && getAssetType($asset) != 'Design_Css') {
			// Then, if it has dependant children, we add those too
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design' || getAssetType($asset) == 'Design_Css') {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design_Customisation') {
			// if we are dealing with customisations, then only deal with
			// the design areas that are customised rest of the design areas
			// will be generated once Matrix processes the parse file
			$dependants = $asset->getCustomisedAreas();
			// now this customistation can further have customisations
			// take care of those ones too in our export
			$dependants = array_merge($dependants, $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, 'design_customisation'));
		}

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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);

		foreach ($notice_links as $notice_link) {
			if (strpos($notice_link['minorid'], ':')) continue;
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

		$dependants = Array();
		if (getAssetType($asset) != 'Design_Customisation' && getAssetType($asset) != 'Design') {
			// Then, if it has dependant children, we add those too
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design') {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design_Customisation') {
			// if we are dealing with customisations, then only deal with
			// the design areas that are customised rest of the design areas
			// will be generated once Matrix processes the parse file
			$dependants = $asset->getCustomisedAreas();
			// now this customistation can further have customisations
			// take care of those ones too in our export
			$dependants = array_merge($dependants, $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, 'design_customisation'));
		}

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
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);

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

		$dependants = Array();
		if (getAssetType($asset) != 'Design_Customisation' && getAssetType($asset) != 'Design') {
			// Then, if it has dependant children, we add those too
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design') {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design_Customisation') {
			// if we are dealing with customisations, then only deal with
			// the design areas that are customised rest of the design areas
			// will be generated once Matrix processes the parse file
			$dependants = $asset->getCustomisedAreas();
			// now this customistation can further have customisations
			// take care of those ones too in our export
			$dependants = array_merge($dependants, $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, 'design_customisation'));
		}

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


	/**
	* Saves the file on the file system to keep a copy for our import.
	* if the asset id provided the information about the file like 'name', 'path'
	* is grabbed from it or else we need to provide it in an array form
	*
	* @param object	$asset		the (file type )asset we are saving file for
	* @param array	$file		the info about the file we are trying to copy
	*
	* @return string
	* @access public
	*/
	function _saveFileAsset(&$asset, $file=Array())
	{
		if (!is_null($asset)) {
			$file_info = $asset->getExistingFile();
			$file_type = getAssetType($asset);
		} else {
			$file_info = $file;
			$file_type = $file['type'];
		}
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


	/**
	* this function will parse for any assetids in the value string 
	* and replace them with our [[output://create_ACTION_.assetid]]
	*
	* @param string	$value				the string we want to get parsed
	* @param string	$attribute_name		the name of the attribute we are processing value for
	* @param string	$actionid			actionid of the attribute value we are trying to set
	*
	* @return string
	* @access public
	*/
	function _parseValue($value, $attribute_name, $actionid)
	{
		global $asset_id_map;
		global $warned;
		$assetid_mapped = Array();

		$shadow_reg = '|.\/?a=(\d+:\w*)|';
		$normal_reg = '|.\/?a=(\d+)|';
		$shadow_matches = Array();
		$normal_matches = Array();

        preg_match_all($shadow_reg, $value, $shadow_matches);
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

		foreach ($replace_assetids as $replace_assetid) {
			if (isset($asset_id_map[$replace_assetid])) {
				$value = preg_replace('|.\/?a='.$replace_assetid.'|', '?a=[[output://create_'.$asset_id_map[$replace_assetid].'.assetid]]', $value);
				$assetid_mapped[] = $replace_assetid;
			}
		}

		$globals_processed = FALSE;
		// we have got the /?a=XX type assetids to be replaced
		// lets not leave away the %globals_xxxxxx_xxx:assetid%
		$kywrd_reg_global_shdw = '|%globals_asset_[a-z]+:(\d+:\w*)%|';
		$kywrd_reg_global_real = '|%globals_asset_[a-z]+:(\d+)%|';
		preg_match_all($kywrd_reg_global_shdw, $value, $globals_match);
		preg_match_all($kywrd_reg_global_real, $value, $globals_match_2);

		if(isset($globals_match[1]))$globals_match = $globals_match[1];
		if(isset($globals_match_2[1]))$globals_match_2 = $globals_match_2[1];

		$replace_globals_assetids = Array();
		foreach ($globals_match as $data) {
				$replace_globals_assetids[] = $data;
		}
		foreach ($globals_match_2 as $data) {
				$replace_globals_assetids[] = $data;
		}

		foreach ($replace_globals_assetids as $replace_globals_assetid) {
			if (isset($asset_id_map[$replace_globals_assetid])) {
				$value = preg_replace('|%globals_asset_([a-z]+):'.$replace_globals_assetid.'%|',
									  '%globals_asset_$1:[[output://create_'.$asset_id_map[$replace_globals_assetid].'.assetid]]',
									   $value);
				$assetid_mapped[] = $replace_globals_assetid;
				$globals_processed = TRUE;
			}
		}

		// lastly if we are adding the 'additional_get' attribute
		// check for the directly mentioned assetids
		// do this only if we have NOT processed globals keywords
		if (!$globals_processed && $attribute_name != 'html' && $attribute_name != 'quiz_answers' && $attribute_name != 'statuses') {
			preg_match_all('/\'([0-9]+)\'/', $value, $assetid_match);
			if (isset($assetid_match[1])) {
				foreach (array_unique($assetid_match[1]) as $assetid) {
					// check to see
					//	- the assetid isnt 0
					//	- the asset exists on the system
					//	- and also that we are importing it
					// if any of the above fails we can be certain that it is something we do nto want
					if ($assetid != '0' && $GLOBALS['SQ_SYSTEM']->am->assetExists($assetid) && array_key_exists($assetid, $asset_id_map)) {
						$value = preg_replace('/'.$assetid.'/', '[[output://create_'.$asset_id_map[$assetid].'.assetid]]', $value);
						$assetid_mapped[] = $assetid;
					}
				}
			}
		}// end if

		if(!empty($assetid_mapped)) {
			if(!$warned) {
				echo_text("\n\n\nWARNING : Matrix has found following Assetid(s) appearing in attribute value and mapped. This should be reviewed !!!\n");
				$warned = TRUE;
			}

			$mapped_str = implode('", "', $assetid_mapped);
			$echo_str = '"'.$mapped_str.'" for Action ID "'.$actionid.'" to set attribute "'.$attribute_name.'"'."\n";
			echo_text($echo_str);
		}

		return $value;

	}//end _parseValue()


	function printMetadataXML($asset_id) {

		global $asset_id_map;

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$assets_done = Array();

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		foreach($mm->getSchemas($asset_id) as $schema_id => $granted) {
			$schema_info = $mm->getAssetSchemaInfo($asset_id, $schema_id);
			echo "<action>\n";
			echo "   <action_id>set_".$asset_id_map[$asset_id]."_metadata_schema_".$schema_id."</action_id>\n";
			echo "   <action_type>set_metadata_schema</action_type>\n";
			echo "   <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
			echo "   <schemaid>".(isset($asset_id_map[$schema_id]) ? "[[output://create_".$asset_id_map[$schema_id].".assetid]]" : $schema_id)."</schemaid>\n";
			echo "   <granted>".$schema_info['granted']."</granted>\n";
			echo "   <cascades>".$schema_info['cascades']."</cascades>\n";
			echo "</action>\n\n";

			$metadata = $mm->getMetadata($asset_id, $schema_id);
			foreach ($metadata as $field_id => $field_info) {
				$field_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($mm->getFieldAssetIdFromName($asset_id, $field_info[0]['name']));
				if($field_asset->type() == 'metadata_field_text') {
					$value = $field_info[0]['value'];
					if (strtolower(SQ_CONF_DEFAULT_CHARACTER_SET) != 'utf-8') {
						$result = '';
						// escape everything else (chars > 126)
						for ($i = 0; $i < strlen($value); ++$i) {
							$ord = ord($value[$i]);
							if ($ord > 126) {
								$result .= '&#'.$ord.';';
							} else {
								$result .= $value[$i];
							}
						}// end for
						if ($result !== '') $value = $result;
						$value = preg_replace('/(\r\n)+/', '<br/>' ,$value);
					}
					$field_info[0]['value']= $value;
				}

				echo "<action>\n";
				echo "   <action_id>set_".$asset_id_map[$asset_id]."_metadata_field_".$field_id."</action_id>\n";
				echo "   <action_type>set_metadata_value</action_type>\n";				
				echo "   <asset>[[output://create_".$asset_id_map[$asset_id].".assetid]]</asset>\n";
				echo "   <fieldid>".(isset($asset_id_map[$field_id]) ? "[[output://create_".$asset_id_map[$field_id].".assetid]]" : $field_id)."</fieldid>\n";
				echo "   <value><![CDATA["._escapeCDATA($field_info[0]['value'])."]]></value>\n";
				echo "</action>\n\n";
			} // end foreach metadata
		} // end foreach schema

		$dependants = Array();
		if (getAssetType($asset) != 'Design_Customisation' && getAssetType($asset) != 'Design') {
			// Then, if it has dependant children, we add those too
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design') {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, '', FALSE, 'major', NULL, TRUE);
		} else if (getAssetType($asset) == 'Design_Customisation') {
			// if we are dealing with customisations, then only deal with
			// the design areas that are customised rest of the design areas
			// will be generated once Matrix processes the parse file
			$dependants = $asset->getCustomisedAreas();
			// now this customistation can further have customisations
			// take care of those ones too in our export
			$dependants = array_merge($dependants, $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_BACKEND_NAV, 'design_customisation'));
		}

		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printMetadataXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					printMetadataXML($link_info['minorid']);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printMetadataXML()


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

	}//end echo_text()

	/**
	* Replace a current system asset's id with its name, because new system might have different asset id. a name will be safer
	*
	* @param string		$s assetid
	*
	* @return void|String
	* @access public
	*/
	function replace_system_assetid($assetid)
	{
		foreach($GLOBALS['SQ_SYSTEM']->am->_system_assetids as $asset_name => $asset_id){
			if($assetid == $asset_id) {
				return $asset_name;
			}
		}
		return NULL;

	}// end replace_system_assetid()
	
	
	/**
	* 
	* Test if a string is serialized
	* @param string	
	*
	* @return boolean
	* @access public
	*/
	function isSerialized($str)
	{
   	 	return ($str == serialize(false) || @unserialize($str) !== false);

	}// end isSerialized()


	/**
	* If the value string contains CDATA section, escape it
	*
	* @param string		$value
	*
	* @return string
	* @access public
	*/
	function _escapeCDATA($value)
	{
		return preg_replace('|(<\!\[CDATA\[.*?)\]\]\>|ms', '$1]]]]><![CDATA[>', $value);

	}// end _escapeCDATA()


	/**
	* copies parse file from old design to newer one
	*
	* @param string		$value
	*
	* @return string
	* @access public
	*/
	function _updateParseFileForDesign($design_id, $parse_file_path)
	{
		$file_info = Array(
						'filename'	=> $design_id.'_parse.txt',
						'path'		=> $parse_file_path,
						'type'		=> 'txt_file',
					 );

		$null_asset = NULL;

		// lets copy the parse file to a location and 
		// then add an action to add it to our new design asset
		$new_file_path = _saveFileAsset($null_asset, $file_info);

		if ($new_file_path != '') {
			echo "<action>\n";
			echo "   <action_id>set_".$design_id."_parse_file</action_id>\n";
			echo "   <action_type>set_design_parse_file</action_type>\n";				
			echo "   <asset>[[output://create_".$design_id.".assetid]]</asset>\n";
			echo "   <file_path>".$new_file_path."</file_path>\n";
			echo "</action>\n\n";
		}

	}// end _updateParseFileForDesign()


?>
