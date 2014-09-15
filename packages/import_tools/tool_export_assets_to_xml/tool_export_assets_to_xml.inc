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
* $Id: tool_export_assets_to_xml.inc,v 1.9 2013/04/23 02:09:39 ewang Exp $
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
* @version $Revision: 1.9 $
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
		// do a lock check, only allows one user at a time
		$import_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
		$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($import_mgr->id, 'attributes', TRUE, TRUE);
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$userHasLockId = '';
		$hasLock = TRUE;

		foreach ($locks as $lock) {
		    if(empty($lock)) continue;
		    if($lock['lock_type'] == 'attributes' && $lock['userid'] == $userid) {
			$hasLock = TRUE;
		    }
		    else if ($lock['lock_type'] == 'attributes' ) {
			$hasLock = FALSE;
			$userHasLockId = $lock['userid'];
		    }
		}

	    	if(!$hasLock) {
			$o->openRaw();
			echo (sprintf(translate('tool_export_assets_to_xml_can_not_acquire_lock'), $userHasLockId));
			$o->closeRaw();
			return FALSE;

		}

		$show_download = FALSE;
		if(isset($_REQUEST['show_download'])) {
			$show_download = TRUE;
		}
		else if (isset($_REQUEST['download'])) {
			$export_dir = SQ_DATA_PATH.'/private/export_assets';
			if (!file_exists($export_dir.'/export.tgz')) {
			 if (!create_directory($export_dir)) return FALSE;

				$command = "cd $export_dir ; tar -czf export.tgz *";
				exec($command, $output, $return);

				// exec populate $return with 0 if successful. If not successful, $return will be an exit status code and
				// we don't have a usable compressed file for the user to download
				if ($return !== 0) {
					trigger_localised_error('EXP0001', E_USER_ERROR, $return);
					return;
				}
			}

			// get the first export root asset id
			$root_id = '';
			$export_xml_file = $export_dir.'/export.xml';
			if(is_file($export_xml_file)) {
			    $handle = @fopen($export_xml_file, "r");
			    if ($handle) {
				while (($buffer = fgets($handle, 4096)) !== false) {
				    if(strpos($buffer, 'action_id') !== FALSE) {
					$matches = Array();
					preg_match('/_([0-9]+)</', $buffer, $matches);
					$root_id = isset($matches[1]) ? '-'.$matches[1] : '';
					break;
				    }
				}
				fclose($handle);
			    }
			}


			// if we don't have a valid filesize, there isn't much point going fruther as the user will end up downloading an unusable file
			$filesize = filesize($export_dir.'/export.tgz');
			if (!$filesize) {
				trigger_localised_error('EXP0002', E_USER_ERROR);
				return;
			}


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
			header('Content-Disposition: attachment; filename="export'.$root_id.'.tgz";');
			header('Content-Length: '.$filesize);

			 // Get the source file
			self::_readfile_chunked($export_dir.'/export.tgz');

			// release lock after user downloads the tarball, otherwise locks will be left to lock system to decide
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($import_mgr->id, 'attributes');
			return;
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
		$import_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');
		if(!$GLOBALS['SQ_SYSTEM']->am->acquireLock($import_mgr->id, 'attributes')) {
			trigger_localised_error('EXP0003', E_USER_ERROR);
			return FALSE;
		}

		// make export temp directory
		$export_dir = SQ_DATA_PATH.'/private/export_assets';
		if (!create_directory($export_dir)) return FALSE;

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



	/**
	* read file in chunks in case of memory exaust
	*
	* @param object	$filename	file name to read
	* @param string	$retbytes	return int of bytes read
	* @param string	$chunksize size of chunk in bytes
	*
	* @return boolean | int
	* @access private
	*/
	 private static function _readfile_chunked($filename, $retbytes = TRUE, $chunksize = 1048576) {
		$buffer = '';
		$cnt =0;
		// $handle = fopen($filename, 'rb');
		$handle = fopen($filename, 'rb');
		if ($handle === false) {
		  return false;
		}
		while (!feof($handle)) {
		  $buffer = fread($handle, $chunksize);
		  echo $buffer;
		  ob_flush();
		  flush();
		  if ($retbytes) {
		    $cnt += strlen($buffer);
		  }
		}
		$status = fclose($handle);
		if ($retbytes && $status) {
		  return $cnt; // return num. bytes delivered like readfile() does.
		}
		return $status;
	  } // end _readfile_chunked()



}//end class


?>
