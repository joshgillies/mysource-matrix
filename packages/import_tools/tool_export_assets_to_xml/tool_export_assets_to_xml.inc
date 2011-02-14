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
* $Id: tool_export_assets_to_xml.inc,v 1.1.2.2 2011/02/14 00:20:44 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Tool_Import_Assets_From_Xml
*
* Purpose
*
*    Tool for exporting assets into xml
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/

class Tool_Export_Assets_To_Xml extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Export_Assets_To_Xml($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return string
	* @access public
	*/
	public static function paintTool(&$o, $type_code)
	{
		$show_download = FALSE;
		if(isset($_REQUEST['show_download'])) {
			$show_download = TRUE;
		}
		else if (isset($_REQUEST['download'])) {
			$export_dir = SQ_DATA_PATH.'/private/export_assets';
			$command = '
			cd '.$export_dir.'
			tar -czf export.tgz * --remove-files';
			system($command);
		
			
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
			$ext = 'tgz';
			$type	= (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];
	
			// Fix for IE caching
			header("Pragma: public");
			header("Expires: 0");
			header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
			header("Cache-Control: private",false);
	
			// We'll be outputting a file
			header('Content-Type: '.$type);
	
			// Set the name of the file
			header('Content-Disposition: attachment; filename="export.tgz";');
			header('Content-Length: '.filesize($export_dir.'/export.tgz'));
	
	        // Get the source file
	        readfile($export_dir.'/export.tgz');
			@ob_flush();
			return;
		}
		

		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField(translate('tool_export_assets_to_xml_root_node'));
			asset_finder($type_code.'_root_node[0]', 0, Array('asset' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
			echo '<span id ="tool_export_assets_to_xml_mapping" style="margin-left:10px;">';
			echo translate('tool_export_assets_to_xml_target_parent');
			text_box($type_code.'_target_parent[0]', '1', '1', '', '', 'style="margin-left:10px;"');
			echo '</span>';
			$map_frame_exp = trim('parent.frames["sq_sidenav"]', '.');
			$name =$type_code.'_root_node';
			$safe_name = 'sq_asset_finder_'.preg_replace('/[^A-Za-z0-9]/', '_', $name);
			?>
		<script type="text/javascript">
		// insert target parent text box when clicking More button
		var target_parent_index = 1;
		function addParentNode(node) {
			var new_span = document.getElementById('tool_export_assets_to_xml_mapping').cloneNode(true);
			new_span.lastChild.setAttribute('id', '<?php echo $type_code; ?>' + '_target_parent[' + target_parent_index + ']');
			new_span.lastChild.setAttribute('name', '<?php echo $type_code; ?>' + '_target_parent[' + target_parent_index + ']');
			node.parentNode.insertBefore(new_span, node);
			target_parent_index = target_parent_index + 1;
		}
		</script>
		<?php 
			echo '<input type="button" name="'.htmlspecialchars($safe_name.'_more_btn').'" id="'.htmlspecialchars($safe_name.'_more_btn').'" value="'.translate('more').'..." onclick="addNewAssetFinder(this, \''.$name.'\', \''.$safe_name.'\', \'\', \''.htmlentities($map_frame_exp, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'\', \'null\', true); addParentNode(this);" style="margin-left:10px;"/>';
			$o->note(translate('tool_export_assets_to_xml_root_node_note'));
		$o->closeField();
		
		if($show_download) {
			$o->openField(translate('download'));
			$backend_url = $GLOBALS['SQ_SYSTEM']->backend->getBackendUrl();
			$download_url = $backend_url.'&backend_section=tools&tool_type_code='.$type_code.'&download=1';
				echo '<a href="'.$download_url.'" >'.translate('download').'</a>';
			$o->closeField();
		}

		$o->closeSection();

		
		
		
	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	public static function processTool(&$o, $type_code)
	{

		// make export temp directory
		$export_dir = SQ_DATA_PATH.'/private/export_assets';
		if (!is_dir($export_dir))
		{
			mkdir($export_dir);
		} 
		
		// clean up before export				
		$command = "rm -rf $export_dir/*"; 
		system($command);
		$root_node_mapping = Array();
		foreach ($_POST[$type_code.'_root_node'] as $index => $node) {
			$node_id = $node['assetid'];
			$target_parentid = 	$_POST[$type_code.'_target_parent'][$index];
			if(empty($node_id) || empty($target_parentid)) continue;
			$root_node_mapping[] = $node_id.':'.$target_parentid;
			
		}
		
		if (empty($root_node_mapping)) {
			trigger_error ('There is no root node mapping specified');	
			return FALSE;
		}
		// set HIPO running_vars
		$vars['export_path'] = $export_dir;
		$vars['root_node_mapping'] = $root_node_mapping;

		// run HIPO job

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_export_assets_to_xml', $vars, '', SQ_PACKAGES_PATH.'/import_tools/hipo_jobs');
		$backend_url = $GLOBALS['SQ_SYSTEM']->backend->getBackendUrl();
		// run the hipo job and redirect back to the tool and show download link
		$url = $hh->runQueuedJobs($backend_url.'&backend_section=tools&tool_type_code='.$type_code.'&show_download=1');
		
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;


	}//end processTool()


}//end class


?>
