<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id:
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* HIPO for exporting assets to xml
*
* Initial Running Vars :
*			'export_path' 			- Full path to temp export directory
*			'root_node'				- Root asset ID to export from
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1.8.2 $
* @package MySource_Matrix
* @subpackage import_tools/hipo_jobs
*
*/
class HIPO_Job_Export_Assets_To_Xml extends HIPO_Job
{


	public $import_action_outputs = Array();

	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Export_Assets_To_Xml($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique id for this job
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return 'HIPO_Job_Export_Assets_To_Xml-'.md5($this->_running_vars['root_node_mapping'].$this->_running_vars['export_path']);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('tool_export_assets_to_xml_hipo_name');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('tool_export_assets_to_xml_hipo_name'),
					'function_call'	=> Array(
										'process_function'	=> 'processExportAsset',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'header_set'	=> FALSE,
				),
			  );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data['complete'] = FALSE;
		while(!$step_data['complete']) {
			if($this->processExportAsset($step_data, '')) {
				return FALSE;
			}
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// 5 components
		$this->_running_vars['sections'] = Array('create', 'attribute', 'metadata', 'link', 'permission');
		// a temp array to record all imported asset ids
		$this->_running_vars['asset_id_map'] = Array();

		return parent::prepare();

	}//end prepare()


	/**
	* Export assets to xml
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processExportAsset(&$step_data, $prefix)
	{
		$path = $this->_running_vars['export_path'];
		$asset_infos = $this->_running_vars['root_node_mapping'];
		$initial_link_type = SQ_LINK_TYPE_1;
		$action = array_shift($this->_running_vars['sections']);
		ob_start();
		if (!$step_data['header_set']) {
			echo "<?xml version=\"1.0\" encoding=\"".SQ_CONF_DEFAULT_CHARACTER_SET."\"?>\n";
			$step_data['header_set'] = TRUE;
		}
		switch ($action) {
			case 'create':
				echo "<actions>\n";
				foreach($asset_infos as $asset_info) {
					$asset_from_to_id = explode(':',$asset_info);
					if(!isset($asset_from_to_id[0]) || !isset($asset_from_to_id[1])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
					$this->printCreateXML($asset_from_to_id[0], $asset_from_to_id[1], $initial_link_type);
				}
				$step_data['percent_done'] = 20;
				$step_data['complete']     = FALSE;
			break;	
			case 'attribute':
				foreach($asset_infos as $asset_info) {
					$asset_from_to_id = explode(':',$asset_info);
					if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
					$this->printAttributeXML($asset_from_to_id[0]);
				}
				$step_data['percent_done'] = 40;
				$step_data['complete']     = FALSE;
			break;
			case 'metadata':
				foreach($asset_infos as $asset_info) {
					$asset_from_to_id = explode(':',$asset_info);
					if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
					$this->printMetadataXML($asset_from_to_id[0]);
				}
				$step_data['percent_done'] = 60;
				$step_data['complete']     = FALSE;
			break;
			case 'link':
				foreach($asset_infos as $asset_info) {
					$asset_from_to_id = explode(':',$asset_info);
					if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
					$this->printNoticeLinksXML($asset_from_to_id[0]);
				}
				$step_data['percent_done'] = 80;
				$step_data['complete']     = FALSE;
			break;	
			case 'permission':
				foreach($asset_infos as $asset_info) {
					$asset_from_to_id = explode(':',$asset_info);
					if(!isset($asset_from_to_id[0])) 	trigger_error("Failed to parse second argument\n", E_USER_ERROR);
					$this->printPermissionXML($asset_from_to_id[0]);
				}			
				echo "</actions>\n\n";
				$step_data['percent_done'] = 100;
				$step_data['complete']     = TRUE;
			break;		
		}

		$output = ob_get_clean();
		
		file_put_contents($path.'/export.xml', $output, FILE_APPEND | LOCK_EX);
	
		
		return TRUE;

	}//end processExportAsset()
	
	
	
	
	
	/**
	* Lovely recursing function to create the XML
	*
	* @param string	$asset_id		id of the asset to create
	* @param string	$parent			parent id to create underneath
	* @param string	$link_type		link type of the new link
	* @param string	$value			link value
	* @param string	$is_dependant 	is dependant
	* @param string	$is_exclusive	is exclusive
	*
	* @return void
	* @access public
	*/
	function printCreateXML($asset_id, $parent, $link_type, $value='', $is_dependant=0, $is_exclusive=0) {


		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		// system assets are not allowed to be exported.
		if ($this->replace_system_assetid($asset->id) != NULL)	trigger_error("Can not export system asset!\n", E_USER_ERROR);


		$action_code = $this->getAssetType($asset).'_'.$asset->id;

		$this->_running_vars['asset_id_map'][$asset->id] = $action_code;

		echo "<action>\n";
		echo "   <action_id>create_".$action_code."</action_id>\n";

		if ($GLOBALS['SQ_SYSTEM']->am->isTypeDecendant($this->getAssetType($asset), 'file')) {
			$file_path = $this->_saveFileAsset($asset);
			echo "   <action_type>create_file_asset</action_type>\n";
			echo "   <file_path>".$file_path."</file_path>\n";
		} else {
			echo "   <action_type>create_asset</action_type>\n";
		}

		echo "   <type_code>".$this->getAssetType($asset)."</type_code>\n";
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
				if (array_key_exists($link_info['minorid'], $this->_running_vars['asset_id_map'])) {
					$this->printLinkXML($parent, $link_info, $action_code);
				} else {
					$this->printCreateXML($link_info['minorid'], $parent, $link_info['link_type'], $link_info['value'], $link_info['is_dependant'], $link_info['is_exclusive']);
				}

			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				$parent = '[[output://create_'.$action_code.'.assetid]]';

				if (array_key_exists($link_info['minorid'], $this->_running_vars['asset_id_map'])) {
					$this->printLinkXML($parent, $link_info, $action_code);
				} else {
					$this->printCreateXML($link_info['minorid'], $parent, $link_info['link_type'], $link_info['value'], $link_info['is_dependant'], $link_info['is_exclusive']);
				}

			}
		}


		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printXML

	/**
	* create the link XML
	*
	* @param string	$parent		 	parent asset id
	* @param array	$link_info		an array of the link info including link type, value etc
	* @param string	$action_code	actions code
	*
	* @return void
	* @access public
	*/
	function printLinkXML($parent, $link_info, $action_code) {

		// if we created the asset, get it from array, otherwise, if it's a system asset, we have to use system asset name.
		if (isset($this->_running_vars['asset_id_map'][$link_info['minorid']])){
			$remap_id =	"[[output://create_".$this->_running_vars['asset_id_map'][$link_info['minorid']].".assetid]]"; 
		}
		else {
			//before we use asset id, we should make sure if it's a system asset, we use system asset name instead of id. 
			//because new system may have different system assets ids.
			$system_asset_name = $this->replace_system_assetid($link_info['minorid']);
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

	}// end printLinkXML()

	
	/**
	* create the attrbute XML
	*
	* @param string	$asset_id		 	asset id to export attribute
	*
	* @return void
	* @access public
	*/
	function printAttributeXML($asset_id) {

		$assets_done = Array();

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		// print some attributes
		foreach ($asset->vars as $attr_name => $attr_info) {
			$attr = $asset->getAttribute($attr_name);
			$value = $attr->getContent();
			// unserilize it, we want clean script
			if($this->isSerialized($value) && !empty($value)){
				$value = var_export(unserialize($value),TRUE);
			}
			// if the value is an assetid attribute, we should match it with asset_id_map first
			if (preg_match("/assetid/",$attr_name) && isset($this->_running_vars['asset_id_map'][$value])){
				$value =	"[[output://create_".$this->_running_vars['asset_id_map'][$value].".assetid]]"; 
			}
			$assets_done[$asset->id] = TRUE;

			if (!empty($value)) {
				echo "<action>\n";
				echo "   <action_id>set_".$this->_running_vars['asset_id_map'][$asset_id].'_'.$attr_name."</action_id>\n";
				echo "   <action_type>set_attribute_value</action_type>\n";
				echo "   <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
				echo "   <attribute>".$attr_name."</attribute>\n";
				if ($attr_name == 'html') { $value = $this->_parseValue($value); }
				echo "   <value><![CDATA[".$value."]]></value>\n";
				echo "</action>\n\n";
			}
		}

		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printAttributeXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printAttributeXML($link_info['minorid']);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printAttributeXML()

	
	/**
	* create the notice link XML
	*
	* @param string	$parent		 	asset id to export notice links
	*
	* @return void
	* @access public
	*/
	function printNoticeLinksXML($asset_id) {

		$assets_done = Array();

		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_LINK_NOTICE, '', FALSE, 'major');

		foreach ($notice_links as $notice_link) {
			// if we created the asset, get it from array, otherwise, if it's a system asset, we have to use system asset name.
			if (isset($this->_running_vars['asset_id_map'][$asset_id])){
				$remap_id =	"[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]" ;
			}
			else {
				//before we use asset id, we should make sure if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = $this->replace_system_assetid($asset_id);
				$system_asset_name == NULL ? $remap_id = $asset_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}
			$this->printLinkXML($remap_id, $notice_link, 'notice_'.$asset_id.'_to_'.$notice_link['minorid']);

		}//end foreach

		$assets_done[$asset_id] = TRUE;

		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printNoticeLinksXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printNoticeLinksXML($link_info['minorid']);
				}
			}
		}


		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printNoticeLinksXML()

	
	
	/**
	* create the permision XML
	*
	* @param string	$asset_id		 	asset id to export permission
	*
	* @return void
	* @access public
	*/	
	function printPermissionXML($asset_id) {

		$assets_done = Array();

		// Now, some permissions
		$read_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_READ, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($read_permissions as $permission_id => $permission_granted) {
			// if the asset is created by this script, use asset_id_map	
			if (isset($this->_running_vars['asset_id_map'][$permission_id])){
				$remap_id =	"[[output://create_".$this->_running_vars['asset_id_map'][$permission_id].".assetid]]" ;
			}
			else {
				//if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = $this->replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}
			
		

			// if the action is to deny a previous permission, make sure we will remove the permission first
			// even removing it will possibly fail, it will not affect the result.
			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_read_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
				echo "    <permission>1</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_read_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
			echo "    <permission>1</permission>\n";
			echo "    <granted>".$permission_granted."</granted>\n";
			echo "    <userid>".$remap_id."</userid>\n";
			echo "</action>\n";

		}//end foreach

		$write_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_WRITE, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($write_permissions as $permission_id => $permission_granted) {

			// if the asset is created by this script, use asset_id_map	
			if (isset($this->_running_vars['asset_id_map'][$permission_id])){
				$remap_id =	"[[output://create_".$this->_running_vars['asset_id_map'][$permission_id].".assetid]]" ;
			}
			else {
				//if it's a system asset, we use system asset name instead of id. 
				//because new system may have different system assets ids.
				$system_asset_name = $this->replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}

			// if the action is to deny a previous permission, make sure we will remove the permission first
			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_write_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
				echo "    <permission>2</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_write_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
			echo "    <permission>2</permission>\n";
			echo "    <granted>".$permission_granted."</granted>\n";
			echo "    <userid>".$remap_id."</userid>\n";
			echo "</action>\n";

		}//end foreach

		$admin_permissions = $GLOBALS['SQ_SYSTEM']->am->getPermission($asset_id, SQ_PERMISSION_ADMIN, NULL, FALSE, FALSE, TRUE, TRUE);
		foreach ($admin_permissions as $permission_id => $permission_granted) {

			if (isset($this->_running_vars['asset_id_map'][$permission_id])){
				$remap_id =	"[[output://create_".$this->_running_vars['asset_id_map'][$permission_id].".assetid]]" ;
			}
			else {
				$system_asset_name = $this->replace_system_assetid($permission_id);
				$system_asset_name == NULL ? $remap_id = $permission_id : $remap_id = "[[system://".$system_asset_name."]]" ;
			}

			if($permission_granted == 0) {
				echo "<action>\n";
				echo "    <action_id>set_permission_".$asset_id."_admin_".$permission_id."</action_id>\n";
				echo "    <action_type>set_permission</action_type>\n";
				echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
				echo "    <permission>3</permission>\n";
				echo "    <granted>-1</granted>\n";
				echo "    <userid>".$remap_id."</userid>\n";
				echo "</action>\n";
			}
			echo "<action>\n";
			echo "    <action_id>set_permission_".$asset_id."_admin_".$permission_id."</action_id>\n";
			echo "    <action_type>set_permission</action_type>\n";
			echo "    <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
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
					$this->printPermissionXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset_id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printPermissionXML($link_info['minorid']);
				}
			}
		}


	}//end printPermissionXML()

	
	/**
	* recursively create the metadata XML for export
	*
	* @param string	$asset_id		 	asset id to export metadata
	*
	* @return void
	* @access public
	*/	
	function printMetadataXML($asset_id) {

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$assets_done = Array();

		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
		if (is_null($asset)) exit();

		foreach($mm->getSchemas($asset_id) as $schema_id => $granted) {
			$schema_info = $mm->getAssetSchemaInfo($asset_id, $schema_id);
			echo "<action>\n";
			echo "   <action_id>set_".$this->_running_vars['asset_id_map'][$asset_id]."_metadata_schema_".$schema_id."</action_id>\n";
			echo "   <action_type>set_metadata_schema</action_type>\n";
			echo "   <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
			echo "   <schemaid>".(isset($this->_running_vars['asset_id_map'][$schema_id]) ? "[[output://create_".$this->_running_vars['asset_id_map'][$schema_id].".assetid]]" : $schema_id)."</schemaid>\n";
			echo "   <granted>".$schema_info['granted']."</granted>\n";
			echo "   <cascades>".$schema_info['cascades']."</cascades>\n";
			echo "</action>\n\n";

			$metadata = $mm->getMetadata($asset_id, $schema_id);
			foreach ($metadata as $field_id => $field_info) {
				echo "<action>\n";
				echo "   <action_id>set_".$this->_running_vars['asset_id_map'][$asset_id]."_metadata_field_".$field_id."</action_id>\n";
				echo "   <action_type>set_metadata_value</action_type>\n";				
				echo "   <asset>[[output://create_".$this->_running_vars['asset_id_map'][$asset_id].".assetid]]</asset>\n";
				echo "   <fieldid>".(isset($this->_running_vars['asset_id_map'][$field_id]) ? "[[output://create_".$this->_running_vars['asset_id_map'][$field_id].".assetid]]" : $field_id)."</fieldid>\n";
				echo "   <value><![CDATA[".$field_info[0]['value']."]]></value>\n";
				echo "</action>\n\n";
			} // end foreach metadata
		} // end foreach schema
		
		// Then, if it has dependant children, we add those too
		$dependants = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, TRUE);
		foreach ($dependants as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printMetadataXML($link_info['minorid']);
				}
			}
		}

		// Now let's do the non-dependant children, shall we?
		$children = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_SC_LINK_SIGNIFICANT, '', FALSE, 'major', NULL, 0);
		foreach ($children as $link_info) {
			if (!strpos($link_info['minorid'], ':')) {
				if (!array_key_exists($link_info['minorid'], $assets_done)) {
					$this->printMetadataXML($link_info['minorid']);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

	}//end printMetadataXML()
		
	
	/**
	* export a file asset by copying the physical file to export directory
	*
	* @param string	$asset		the file asset to export
	*
	* @return void
	* @access private
	*/
	private function _saveFileAsset(&$asset) {

		$path = $this->_running_vars['export_path'];
		$file_info = $asset->getExistingFile();
		$file_type = $this->getAssetType($asset);
		$export_path = $path.'/export/'.$file_type.'/'.$file_info['filename'];

		// check to see if an export/ directory exists. If not, create it.
		if (!file_exists($path.'/export/')) {
			mkdir($path.'/export', 0775);
		}

		// echeck to see if the type_code directory exists
		if (!file_exists($path.'/export/'.$file_type.'/')) {
			mkdir($path.'/export/'.$file_type, 0775);
		}

		if (copy($file_info['path'], $export_path)) {
			return 'export/'.$file_type.'/'.$file_info['filename'];
		} else {
			die('Could not copy file');
		}


	}//end _saveFileAsset()

	
	/**
	* process the attribute value for reference of asset ids. e.g ?a=xx links should be replaced if the referred the asset is exported together
	*
	* @param string	$value		the attribute value to process
	*
	* @return void
	* @access private
	*/
	function _parseValue($value) {
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
			if (isset($this->_running_vars['asset_id_map'][$replace_assetid])) {
				$value = preg_replace('|.\/?a='.$replace_assetid.'|', '?a=[[output://create_'.$this->_running_vars['asset_id_map'][$replace_assetid].'.assetid]]', $value);
			}
		}

		return $value;

	}// end _parseValue()


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
	


}//end class

?>
