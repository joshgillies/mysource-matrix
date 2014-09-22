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
* $Id: structured_asset_info_edit_fns.inc,v 1.20 2012/08/30 01:09:09 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* Structured_Asset_Info_Edit_Fns
*
* Edit Functions for the backend interface of asset statuses report
*
*
* @author  Elden McDonald <emcdonald@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Structured_Asset_Info_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for choosing which asset types to list
	*
	* @param Structured_Asset_Info	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintAssetTypes(Structured_Asset_Info $asset, Backend_Outputter $o, $prefix)
	{
		$types = $asset->attr('types');
		if ($asset->writeAccess('attributes')) {
			// print the form fields
			$display_values = Array(
								'type_code'	=> Array(),
								'inherit'	=> Array(),
							  );
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][]   = $inherit;
			}
			asset_type_chooser($prefix.'_types', TRUE, $display_values, NULL, TRUE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('No types have been selected').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Type') ?></th>

							<th><?php echo translate('Inherit?') ?></th>

						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');


					?>
						<tr>
							<td><?php echo $type_names[$type]; ?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}

	}//end paintAssetTypes()


	/**
	* Process the asset types to list
	*
	* @param Structured_Asset_Info	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processAssetTypes(Structured_Asset_Info $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_types'])) {
			$existing_types = $asset->attr('types');
			$submitted_types = Array();
			foreach ($_POST[$prefix.'_types']['type_code'] as $i => $type_code) {
				if (!empty($type_code)) {
					$submitted_types[$type_code] = $_POST[$prefix.'_types']['inherit'][$i];
				}
			}
			$asset->setAttrValue('types', $submitted_types);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processAssetTypes()


	/**
	* Generate the value of the Assetid field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldAssetid(Array $asset_info)
	{
		return $this->_cData($asset_info['assetid']);

	}//end _generateFieldAssetid()


	/**
	* Generate the value of the Status field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldStatus(Array $asset_info)
	{
		return $asset_info['status'];

	}//end _generateFieldStatus()


	/**
	* Generate the value of the AssetType field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldAssetType(Array $asset_info)
	{
		return $this->_cData($asset_info['type_code']);

	}//end _generateFieldAssetType()


	/**
	* Generate the value of the Name field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldName(Array $asset_info)
	{
		return $this->_cData($asset_info['name']);

	}//end _generateFieldName()


	/**
	* Generate the value of the CreateDate field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldCreatedDate(Array $asset_info)
	{
		return $asset_info['created'];

	}//end _generateFieldCreatedDate()


	/**
	* Generate the value of the CreateUser field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldCreatedUser(Array $asset_info)
	{
		return $asset_info['created_userid'];

	}//end _generateFieldCreatedUser()


	/**
	* Generate the value of the LastUpdatedDate field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastUpdatedDate(Array $asset_info)
	{
		return $asset_info['updated'];

	}//end _generateFieldLastUpdatedDate()


	/**
	* Generate the value of the LastUpdatedUser field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastUpdatedUser(Array $asset_info)
	{
		return $asset_info['updated_userid'];

	}//end _generateFieldLastUpdatedUser()


	/**
	* Generate the value of the LastPublishedDate field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastPublishedDate(Array $asset_info)
	{
		return $asset_info['published'];

	}//end _generateFieldLastPublishedDate()


	/**
	* Generate the value of the LastPublishedUser field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastPublishedUser(Array $asset_info)
	{
		return $asset_info['published_userid'];

	}//end _generateFieldLastPublishedUser()


	/**
	* Generate the value of the LastChangeDate field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastStatusChangeDate(Array $asset_info)
	{
		return $asset_info['status_changed'];

	}//end _generateFieldLastStatusChangeDate()


	/**
	* Generate the value of the LastChangeUser field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldLastStatusChangeUser(Array $asset_info)
	{
		return $asset_info['status_changed_userid'];

	}//end _generateFieldLastStatusChangeUser()


	/**
	* Generate the value of the Urls field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldUrls(Array $asset_info)
	{
		$value = '';
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_info['assetid']);
		$urls = $asset->getLookups();
		if (!empty($urls)) {
			usort($urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
			foreach ($urls as $data) {
				if ($data['http']) {
					$value .= '<url>http://'.$data['url'].'</url>'."\n";
				}
				if ($data['https']) {
					$value .= '<url>https://'.$data['url'].'</url>'."\n";
				}
			}
		}
		return $value;

	}//end _generateFieldUrls()


	/**
	* Generate the value of the Designs field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldDesigns(Array $asset_info)
	{
		$value = '';
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_info['assetid']);
		$designs = $asset->getDesignLookups();
		ksort($designs);
		/*
		example xml, see _paintFieldDesigns() for example data
			<designs>
				<design url="url_one">
					<design_type>
						<type><![CDATA[design::user::printer friendly]]></type>
						<id><![CDATA[88]]></id>
					</design_type>
					<design_type>
						<type><![CDATA[design::system::frontend]]></type>
						<id><![CDATA[91]]></id>
					</design_type>
					<design_type>
						<type><![CDATA[design::system::login]]></type>
						<id><![CDATA[41]]></id>
					<design_type>
				</design>
			</designs>
		*/
		foreach ($designs as $url => $data) {
			// store all design applied to this URL to the report
			$value .= '<design url="'.$url.'">'."\n";
			foreach ($data as $type => $id) {
				$value .= '<design_type>';
				$value .= '<type>'.$this->_cData($type).'</type>';
				$value .= '<id>'.$this->_cData($id).'</id>';
				$value .= '</design_type>'."\n";
			}
			$value .= "</design>\n";
		}
		return $value;

	}//end _generateFieldDesigns()


	/**
	* Generate the value of the MetadataSchemas field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldMetadataSchemas(Array $asset_info)
	{
		$value = '';
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($asset_info['assetid'], TRUE);
		foreach ($schemas as $schemaid) {
			// get the name of the schema, put it into the tag pre-formatted
			$value .= '<schema>'.$schemaid.'</schema>'."\n";
		}
		return $value;

	}//end _generateFieldMetadataSchemas()


	/**
	* Generate the value of the WorkflowSchemas field
	*
	* @param array	$asset_info	Array returned by calling getAssetInfo for the current asset
	*
	* @return string
	* @access private
	*/
	protected function _generateFieldWorkflowSchemas(Array $asset_info)
	{
		$value = '';
		$wm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wm->getSchemas($asset_info['assetid'], TRUE);
		foreach ($schemas as $schemaid) {
			// get the name of the schema, put it into the tag pre-formatted
			$value .= '<schema>'.$schemaid.'</schema>'."\n";
		}
		return $value;

	}//end _generateFieldWorkflowSchemas()


	/**
	* Wraps the get_asset_tag_line function in an empty check
	*
	* @param string	$assetid	the asset to print
	*
	* @return string
	* @access private
	*/
	protected function _printAssetTagLine($assetid)
	{
		$value = '';
		if (!empty($assetid)) {
			$value = get_asset_tag_line($assetid);
		}
		return $value;

	}//end _printAssetTagLine()


	/**
	* wrap the passed data in a cdata tag
	*
	* @param string	$character_data	the data to be put in the cdata tag
	*
	* @return string
	* @access private
	*/
	protected function _cData($character_data)
	{
		return '<![CDATA['.$character_data.']]>';

	}//end _cData()


	/**
	* Generate this report until complete
	*
	* @param object	&$job		the relevant HIPO job
	* @param array	&$step_data	a reference to the array of information about the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	public function generateReport(HIPO_Job $job, Array &$step_data, $prefix)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$owner = $am->getAsset($job->_running_vars['assetid']);

		if (!isset($job->_running_vars['assets_to_list'])) {
			// this is the first step

			// get the selected fields and store them in the running vars
			if ($owner->attr('selected_asset_fields') == '') {
				trigger_localised_error('RPRT0003', E_USER_WARNING);
			}
			$job->_running_vars['selected_asset_fields'] = explode('|', $owner->attr('selected_asset_fields'));


			// get the immediate children of the root to start the stack
			$assets_to_list	= Array();

			$root_id = $this->getRootAssetid($owner);
			if (!$root_id) {
				trigger_localised_error('RPRT0004', E_USER_WARNING);
				return FALSE;
			}

			$child_links = $am->getLinks($root_id, SQ_SC_LINK_BACKEND_NAV);
			foreach (array_reverse($child_links) as $link) {
				if (($link['linkid'] != '0') && FALSE === strpos($link['linkid'], ':')) {
					array_unshift($assets_to_list, $link['minorid']);
				}
			}

			$job->_running_vars['assets_to_list'] = $assets_to_list;
			$job->_running_vars['assets_open'] = Array();
			$job->_running_vars['assets_skipped'] = Array();

			// find the total number of children that we will eventually process
			$asset_treeids = $am->getAssetTreeids($root_id);
			$root_treeid = reset($asset_treeids);
			$root_treeid = $root_treeid === '-' ? '' : $root_treeid;

			try {
				$bind_vars['treeid'] = $root_treeid.'_%';
				$total = MatrixDAL::executeOne('structured_asset_info', 'linkTreeCount', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to get link tree count for "'.$root_treeid.'": '.$e->getMessage());
				$total = 0;
			}

			// double the number of steps to-do because two steps are required for each asset
			$job->_running_vars['total'] = $total*2;
			$job->_running_vars['done'] = 0;


			// prepare a directory in which to write the log file
			$report_dir = $owner->data_path;
			$xml_file_name = $report_dir.'/report.tmp';

			if (!is_dir($report_dir)) {
				if (!create_directory($report_dir)) {
					trigger_localised_error('CORE0198', E_USER_WARNING);
					return FALSE;
				}
			}

			// file manipulation section
			$temp_file = fopen($xml_file_name, 'w');
			if ($temp_file === FALSE) {
				trigger_localised_error('CORE0017', E_USER_WARNING);
				return FALSE;
			}
			fwrite($temp_file, '<?xml version="1.0" encoding="'.SQ_CONF_DEFAULT_CHARACTER_SET.'"?>'."\n");
			fwrite($temp_file, '<assets root_treeid="'.$root_treeid.'">'."\n");
		} else {

			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === FALSE) {
				trigger_localised_error('RPRT0000', E_USER_WARNING);
				return FALSE;
			}
		}

		 $assets_per_step 	= 50;
		 $target_this_step	= $job->_running_vars['done'] + $assets_per_step;

		 $nothing_open = Array(
							'assetid'	=> '',
							'treeid'	=> '',
						 );

		while (!empty($job->_running_vars['assets_to_list']) && $job->_running_vars['done'] <  $target_this_step) {
			$asset_info = Array();
			$contents = '';
			$assets_to_list			= $job->_running_vars['assets_to_list'];
			$assets_open			= $job->_running_vars['assets_open'];
			$selected_asset_fields	= $job->_running_vars['selected_asset_fields'];
			$assets_skipped			= $job->_running_vars['assets_skipped'];
			$current_assetid		= $assets_to_list[0];
			// generate XML contents

			$open_asset = array_get_index($assets_open, 0, $nothing_open);

			if ($current_assetid == $open_asset['assetid']) {
				// closing - look up the tree one level
				array_shift($assets_to_list);
				array_shift($assets_open);
				if ($current_assetid == array_get_index($assets_skipped, 0)) {
					array_shift($assets_skipped);
				} else {
					$contents .= '</children>'."\n";
					$contents .= '</asset>'."\n";
				}

			} else {


				$asset_info = $am->getAssetInfo(Array($current_assetid));
				if (isset($asset_info[$current_assetid])) {
					// if getAssetInfo returns empty array, then might be a problem with the shadow asset bridge
					$asset_info = $asset_info[$current_assetid];
				}
				$asset_info['treeid'] = $this->getTreeid($current_assetid, $open_asset['treeid']);
				$asset_info['assetid'] = $current_assetid;

				if (!isset($asset_info['type_code']) || !$this->checkType($asset_info['type_code'], $owner->attr('types'))) {
					// skip this asset - don't display it in the XML, but do check it's children, so add it to the stack
					array_unshift($assets_skipped, $current_assetid);
				} else {

					// starting a new asset
					$contents .= '<asset>'."\n";

					// generate the fields for this asset
					$contents .= '<fields>'."\n";
					$contents .= '<short_name><![CDATA['.$asset_info['short_name'].']]></short_name>';
					$contents .= '<treeid>'.$asset_info['treeid'].'</treeid>';
					if (!in_array('asset_type', $selected_asset_fields)) {
						// add in asset type code as it will be used for other fields
						$contents .= '<asset_type><![CDATA['.$asset_info['type_code'].']]></asset_type>';
					}
					foreach ($selected_asset_fields as $name) {
						$contents .= '<'.$name.'>';
						$function_name = '_generateField'.str_replace(' ', '', ucwords(str_replace('_', ' ', $name)));

						$contents .= $this->$function_name($asset_info);
						$contents .= '</'.$name.'>'."\n";
					}
					$contents .= '</fields>'."\n";

					// open the children section of this asset and prepare to start listing our children on the next iteration
					$contents .= '<children>'."\n";
				}
				$child_links = $am->getLinks($current_assetid, SQ_SC_LINK_BACKEND_NAV);
				// get more assets to list and stack em
				foreach (array_reverse($child_links) as $link) {
					if (($link['linkid'] != '0') && FALSE === strpos($link['linkid'], ':')) {
						array_unshift($assets_to_list, $link['minorid']);
					}
				}
				$new_open_asset = Array(
									'assetid'	=> $asset_info['assetid'],
									'treeid'	=> $asset_info['treeid'],
								  );
				array_unshift($assets_open, $new_open_asset);

			}//end else
			$job->_running_vars['assets_to_list']		= $assets_to_list;
			$job->_running_vars['assets_open']			= $assets_open;
			$job->_running_vars['assets_skipped']		= $assets_skipped;

			fwrite($temp_file, $contents);

			$job->_running_vars['done']++;

		}//end while there are still assets to list

		if (empty($job->_running_vars['assets_to_list'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$job->_running_vars['complete'] = TRUE;
			fwrite($temp_file, '</assets>'."\n");
			// move the temp file to become the real report XML file
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_localised_error('RPRT0001', E_USER_WARNING);
				return FALSE;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_localised_error('RPRT0002', E_USER_WARNING);
			}
		} else {
			$total = $job->_running_vars['total'];
			$step_data['percent_done'] = ($job->_running_vars['done'] / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		fclose($temp_file);
		return TRUE;

	}//end generateReport()


	/**
	* Process a start element from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$tag	tag name
	* @param array	$attrs	tag attributes
	*
	* @return boolean
	* @access public
	*/
	public function startElement($parser, $tag, $attrs)
	{

		$stack			=& $this->_tmp['paint_stack'];
		$current_asset	=& $this->_tmp['current_asset'];
		$depth			=& $this->_tmp['depth'];

		switch ($tag) {
			case 'assets':
				// set any attributes that apply to the whole report
				$this->_tmp['root_treeid'] = $attrs['root_treeid'];
			break;
			case 'fields':
				// reset the current asset
				$current_asset = Array();
			break;
			case 'children':
				$depth++;
			break;
			case 'design':
				// store the url attribute that will be applied to all design types
				$this->_tmp['current_design_url'] = $attrs['url'];
			break;

			default:
				// ignore tag
		}
		// add the open tag to the stack
		array_unshift($stack, $tag);

	}//end startElement()


	/**
	* Process tag contents from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$data	the character data from the parser
	*
	* @return boolean
	* @access public
	*/
	public function tagContent($parser, $data)
	{

		$stack			=& $this->_tmp['paint_stack'];
		$current_asset	=& $this->_tmp['current_asset'];
		$depth			=& $this->_tmp['depth'];

		$open_tag 			= array_get_index($stack, 0);
		$parent_tag			= array_get_index($stack, 1);
		$grandparent_tag	= array_get_index($stack, 2);

		// the parser treats newlines as character data, in which we're not interested.
		if ($data == "\n") return;

		if ($parent_tag == 'fields') {
			// we're a string
			$current_asset[$open_tag] = $data;

		} else if ($grandparent_tag == 'fields') {
			// we're an array
			// use the name of our parent tag because the open tag is only the name of the array element, not the whole array
			if (!is_array(array_get_index($current_asset,$parent_tag))) {
				$current_asset[$parent_tag] = Array();
			}
			array_push($current_asset[$parent_tag], $data);

		} else if ($parent_tag == 'design_type') {
			// construct the design data array
			if ($open_tag == 'type') {
				$this->_tmp['current_design_type'] = $data;
			} else if ($open_tag == 'id') {
				if (!isset($current_asset['designs'][$this->_tmp['current_design_url']])) {
					$current_asset['designs'][$this->_tmp['current_design_url']] = Array();
				}
				$current_asset['designs'][$this->_tmp['current_design_url']][$this->_tmp['current_design_type']] = $data;
				unset($this->_tmp['current_design_type']);
			}
		}

	}//end tagContent()


	/**
	* Process an end element from the XML parser
	*
	* @param object	$parser	the calling xml parser
	* @param string	$tag	tag name
	*
	* @return boolean
	* @access public
	*/
	public function endElement($parser, $tag)
	{
		$stack			=& $this->_tmp['paint_stack'];
		$current_asset	=& $this->_tmp['current_asset'];
		$depth			=& $this->_tmp['depth'];

		switch ($tag) {
			case 'fields':
				// paint the current asset
				$this->_paintAsset($current_asset);
			break;
			case 'children':
				$depth--;
			break;

			default:
				// ignore tag
		}
		// remove the closing tag from the stack
		array_shift($stack);

	}//end endElement()


	/**
	* Paint the interface for viewing a report
	*
	* @param Structured_Asset_Info	$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintReport(Structured_Asset_Info $asset, Backend_Outputter $o, $prefix)
	{

		// start the page - <table> etc
		echo '<table class="sq-backend-table">';

		// print the table header row

		$field_selector = $GLOBALS['SQ_SYSTEM']->am->getAttribute($asset->vars['selected_asset_fields']['attrid']);
		$field_name_translation = $field_selector->getOptions();

		echo '<tr>';
		// first cell contains the short name
		echo '<td class="sq-backend-table-header">Asset</td>';
		$selected_asset_fields = explode('|', $asset->attr('selected_asset_fields'));
		$this->_tmp['selected_asset_fields'] = $selected_asset_fields;
		foreach ($selected_asset_fields as $field) {
				echo '<td class="sq-backend-table-header">'.str_replace(' ','&nbsp;', $field_name_translation[$field]).'</td>';
		}
		echo '</tr>';

		$this->_tmp['paint_stack']		= Array();
		$this->_tmp['current_asset']	= Array();
		$this->_tmp['depth'] 			= 0;


		$xml_parser = xml_parser_create();
		// stop the parser ucasing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_character_data_handler ( $xml_parser, Array(&$this,'tagContent') );
		xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

		// initialise the file pointer
		$report_path = $asset->data_path.'/report.xml';

		// Parse the file

		if (!xml_parse($xml_parser, file_get_contents($report_path), TRUE)) {
			trigger_localised_error('CORE0257', E_USER_WARNING, $report_path);
			return FALSE;
		}

		// finish the page
		echo '</table>';

		return TRUE;

	}//end paintReport()


	/**
	* Paint an asset row in the report
	*
	* @param array	$current_asset	the data describing the asset to be painted
	*
	* @return void
	* @access private
	*/
	protected function _paintAsset(Array $current_asset)
	{
		echo '<tr>';

		// first cell contains the short name indented to indicate the tree structure
		echo '<td class="sq-backend-table-cell">';
			$this->_paintFieldShortName($current_asset['short_name'], $current_asset['asset_type'], $current_asset['treeid']);
		echo '</td>';

		unset($current_asset['short_name']);

		// paint the rest of the fields
		$selected_asset_fields = $this->_tmp['selected_asset_fields'];
		foreach ($selected_asset_fields as $selected_field) {
			$field = array_get_index($current_asset, $selected_field);
			echo '<td class="sq-backend-table-cell">';
				$paint_function = '_paintField'.str_replace(' ', '', ucwords(str_replace('_', ' ', $selected_field)));
				$this->$paint_function($field);
			echo '</td>';
		}
		echo '</tr>';

	}//end _paintAsset()


	/**
	* Paint the ShortName field
	*
	* @param string	$data		the data to paint
	* @param string	$type_code	the type of this asset for icon display
	* @param string	$treeid		the treeid of this asset for indenting
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldShortName($data, $type_code, $treeid)
	{
		$depth = strlen($treeid)/SQ_CONF_ASSET_TREE_SIZE - strlen($this->_tmp['root_treeid'])/SQ_CONF_ASSET_TREE_SIZE ;
		$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
		echo '<div style="margin-left: '.($depth * 10).'px">
				<script type="text/javascript">sq_print_icon("'.$GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($type_code).'", "16", "16", "'.$type_name.'");</script>&nbsp;'.str_replace(' ', '&nbsp;', $data).'
			  </div>';

	}//end _paintFieldShortName()


	/**
	* Paint the Assetid field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldAssetid($data)
	{
		echo $data;

	}//end _paintFieldAssetid()


	/**
	* Paint the Status field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldStatus($data)
	{
		$status_names = get_status_names();
		?><div style="text-align: center; padding: 0 1ex 0 1ex; background: #<?php echo get_status_colour($data); ?>">
			<?php echo $status_names[$data]; ?>
		</div><?php

	}//end _paintFieldStatus()


	/**
	* Paint the AssetType field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldAssetType($data)
	{
		$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($data, 'name');
		?>
			<script type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($data); ?>", "16", "16", "<?php echo $type_name; ?>");</script>
			<?php echo $data; ?>
		<?php

	}//end _paintFieldAssetType()


	/**
	* Paint the Name field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldName($data)
	{
		echo $data;

	}//end _paintFieldName()


	/**
	* Paint the CreatedDate field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldCreatedDate($data)
	{
		echo $data;

	}//end _paintFieldCreatedDate()


	/**
	* Paint the CreatedUser field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldCreatedUser($data)
	{
		echo $this->_printAssetTagLine($data);

	}//end _paintFieldCreatedUser()


	/**
	* Paint the LastUpdatedDate field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastUpdatedDate($data)
	{
		echo $data;

	}//end _paintFieldLastUpdatedDate()


	/**
	* Paint the LastUpdatedUser field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastUpdatedUser($data)
	{
		echo $this->_printAssetTagLine($data);

	}//end _paintFieldLastUpdatedUser()


	/**
	* Paint the LastPublishedDate field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastPublishedDate($data)
	{
		echo $data;

	}//end _paintFieldLastPublishedDate()


	/**
	* Paint the LastPublishedUser field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastPublishedUser($data)
	{
		echo $this->_printAssetTagLine($data);

	}//end _paintFieldLastPublishedUser()


	/**
	* Paint the LastStatusChangeDate field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastStatusChangeDate($data)
	{
		echo $data;

	}//end _paintFieldLastStatusChangeDate()


	/**
	* Paint the LastStatusChangeUser field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldLastStatusChangeUser($data)
	{
		echo $this->_printAssetTagLine($data);

	}//end _paintFieldLastStatusChangeUser()


	/**
	* Paint the Urls field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldUrls($data)
	{
		if (!empty($data)) echo implode("<br />\n", $data);

	}//end _paintFieldUrls()


	/**
	* Paint the Designs field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldDesigns($data)
	{
		if (!empty($data)) {
			/*
			example data, see _generateFieldDesigns() for example xml
				array(
					"url_one" => array (
						"design::user::printer friendly" => "88",
						"design::system::frontend" => "91",
						"design::system::login" => "41"
					)
				)
			*/
			foreach ($data as $url => $designs) {
				echo '<fieldset><legend>'.$url.'</legend>';
				foreach ($designs as $design_name => $design_id) {
					// work out the display name of this design
					$matches = Array();
					preg_match('/^design::(system|user)::(.*)$/', $design_name, $matches);
					$design_type     = $matches[1];
					$design_code     = $matches[2];
					$display_name = sprintf(translate('%1$s Defined %2$s Design'), ucwords($design_type), ucwords(str_replace('_', ' ', $design_code)));

					echo $display_name.': '.$this->_printAssetTagLine($design_id)."<br>\n";
				}
				echo '</fieldset><br>';
			}
		}

	}//end _paintFieldDesigns()


	/**
	* Paint the MetadataSchemas field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldMetadataSchemas($data)
	{
		if (!empty($data)) {
			foreach ($data as $schemaid) {
				echo $this->_printAssetTagLine($schemaid)."<br />\n";
			}
		}

	}//end _paintFieldMetadataSchemas()


	/**
	* Paint the WorkflowSchemas field
	*
	* @param string	$data	the data belonging to the field
	*
	* @return void
	* @access private
	*/
	protected function _paintFieldWorkflowSchemas($data)
	{
		if (!empty($data)) {
			foreach ($data as $schemaid) {
				echo $this->_printAssetTagLine($schemaid)."<br />\n";
			}
		}

	}//end _paintFieldWorkflowSchemas()


	/**
	* Check whether we should report on assets of the given type
	*
	* @param string	$type			the type to check
	* @param array	$allowed_types	the allowed asset types in the format Array(type => 1/0)
	*
	* @return boolean
	* @access private
	*/
	public function checkType($type, Array $allowed_types)
	{
		$whitelist =& $this->_tmp['whitelist'];
		if (!isset($whitelist[$type])) {
			// build a whitelist if this is the first check
			foreach ($allowed_types as $allowed_type => $inherit) {
				if ($inherit) {
					$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($allowed_type, TRUE);
					if (in_array($type, $type_desc)) {
						$whitelist[$type] = TRUE;
						foreach ($type_desc as $descendant) {
							$whitelist[$descendant] = TRUE;
						}
					}
				} else {
					if ($type == $allowed_type) $whitelist[$type] = TRUE;
				}
			}
		}
		return array_get_index($whitelist, $type, FALSE);

	}//end checkType()


	/**
	* Get the treeid for an asset that contains a particular sub tree
	*
	* @param string	$assetid		the assetid of the asset for which to find a treeid
	* @param string	$parent_treeid	a treeid which must be a substring of the returned treeid
	*
	* @return string
	* @access private
	*/
	public function getTreeid($assetid, $parent_treeid)
	{
		try {
			$bind_vars = Array(
							'assetid'	=> $assetid,
							'treeid'	=> $parent_treeid.'_%',
						 );
			$treeid = MatrixDAL::executeOne('structured_asset_info', 'getTreeid', $bind_vars);
		} catch (Exception $e) {
			$treeid = '';
		}

		return $treeid;

	}//end getTreeid()


}//end class

?>
