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
* $Id: hipo_job_tool_export_files.inc,v 1.5 2012/08/30 00:58:08 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to export files to a tar ball
*
* Initial Running Vars :
*    root_node		-	the root node to grab all the assets from
*    save_name		-	the base name of the tar ball
*	 keep_structure	-	keep the structure of the assets in the asset tree
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Tool_Export_Files extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function __construct($code_name='')
	{
		$this->uses_trans = FALSE;
		parent::__construct($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('cms');

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['root_node'].'-'.$this->_running_vars['save_name'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('cms_hipo_name_export_files');

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
					'name'			=> translate('cms_hipo_exporting_files'),
					'function_call'	=> Array(
										'process_function'	=> 'processExport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
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
		while (!empty($this->_running_vars['todo_asset_users'])) {
			if (!$this->processExport($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// add root_nodes into the 'todo' array
		$i = 0;
		$current_root_node = Array();

		// Get the root nodes
		foreach ($this->_running_vars['root_node'] as $each_node_key => $each_node) {
			foreach ($each_node as $node_key => $node_value) {
				if ($node_key == 'assetid') {
					$current_root_node[$each_node_key] = $node_value;
				}//end if
			}//end foreach
		}//end foreach

		foreach ($current_root_node as $current_node) {
			$this->_running_vars['todo_asset_users'][$i]['root_node'] = $current_node;
			$this->_running_vars['todo_asset_users'][$i]['save_name'] = $this->_running_vars['save_name'];
			$this->_running_vars['todo_asset_users'][$i]['save_location'] = $this->_running_vars['save_location'];
			$this->_running_vars['todo_asset_users'][$i]['keep_structure'] = $this->_running_vars['keep_structure'];
			$i++;
		}//end foreach
		$this->_running_vars['done_asset_users'] = Array();
		return parent::prepare();

	}//end prepare()


	/**
	* Export files from a root node to a tar ball
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_asset_users'])) {

			// export files
			$index = array_shift(array_keys($this->_running_vars['todo_asset_users']));
			if (isset($this->_running_vars['todo_asset_users'][$index]['root_node'])) {
				$step_root_node = $this->_running_vars['todo_asset_users'][$index]['root_node'];
			} else {
				return FALSE;
			}
			$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($step_root_node);
			$save_name = $this->_running_vars['todo_asset_users'][$index]['save_name'];
			$save_location = $this->_running_vars['todo_asset_users'][$index]['save_location'];
			$structure = $this->_running_vars['todo_asset_users'][$index]['keep_structure'];

			// Get the name ready
			$filename = $save_name;
			if (!preg_match('/(\.tar\.gz|\.tgz|\.tar)$/i', $filename)) {
				$filename = $filename.'.tar.gz';
			}

			// Process the node
			$result_node = $this->processExportNode($step_root_node, $filename, $save_location, $structure);
			if (!$result_node) {
				trigger_error("Could not process root node: ".$root_node->id);
				return FALSE;
			} else {
				unset($this->_running_vars['todo_asset_users'][$index]);
			}//end if

		}//end if not empty todo

		// update progress
		if (empty($this->_running_vars['todo_asset_users'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_asset_users']) + count($this->_running_vars['done_asset_users']);
			$step_data['percent_done'] =(count($this->_running_vars['done_asset_users']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processExport()


	/**
	* Export files from a root node to a tar ball
	*
	* @param string		$root_node		The root node where the files and folders are going to come from
	* @param string		$save_name		The file name of the exported file (ie. the tar ball)
	* @param string		$save_location	The location to save the exported file (ie. the tar ball)
	* @param boolean	$keep_structure	Do we keep the structure or not
	*
	* @return boolean
	* @access public
	*/
	function processExportNode($root_node, $save_name, $save_location, $keep_structure=TRUE)
	{
		// Change this save location to the path that you want the files in
		// Apache User *MUST* have read AND write access to that directory
		$allowed_filetypes = Array(
								'folder',
								'file',
							 );

		$export_date = date('hisdmY');
		$structure = $keep_structure;
		$files = Array();
		$depth = 0;

		// Gather a list of all the files to do
		$files = $this->_getFiles($root_node, $allowed_filetypes, $depth);

		$tmpdir = mkdir(SQ_TEMP_PATH.'/export-'.$export_date, 0777);
		if (!$tmpdir) {
			trigger_localised_error('CMS0105', E_USER_NOTICE);
			return FALSE;
		}

		// Process the list
		$filelist = Array();
		
		// Replace '/' characters in filenames by '_'
		foreach($files as $key => $value) {					
			$filename = $value['name'];
			$file_path = $value['path'];

			if (strpos($filename, '/') !== FALSE) {
				$value['name'] = str_replace('/' ,'_', $filename);
				$value['path'] = substr($file_path, 0, strlen($file_path) - strlen($filename)).$value['name'];
				
				$files[$key] = $value;
			}
		}

		$files_count = count($files);
		
		// Fix asset's name that are in same tree level with same name to prevent duplicate filename error
		for($file_index = 1; $file_index <= $files_count; $file_index++) {
			$file_val = $files[$file_index];
			
			$recheck_dups = FALSE;
			for($file_index_check = 1; $file_index_check <= $files_count; $file_index_check++) {
				if ($file_index == $file_index_check) {
					continue;
				}
				$file_val_check = $files[$file_index_check];
				
				if ($file_val['root'] == $file_val_check['root'] && $file_val['name'] == $file_val_check['name']) {
					// Append asset's webpath to the asset's name make the file name unique
					$file_val_check['name'] .= '_'.$file_val_check['webpath'];
					$file_val_check['path'] .= '_'.$file_val_check['webpath'];
					
					$files[$file_index_check] = $file_val_check;
					
					// Need to check for duplicates all again to see if this new name is conflicting with others previously checked
					$recheck_dups = TRUE;
				}
			}
			
			if ($recheck_dups) {
				$file_index = 1;
			}
		}
		
		foreach ($files as $key => $value) {
			
			// Build file list
			$path_to = '';
			if ($structure) {
				$path_to = $this->_findPath($files, $value, $path_to);
				$path_to = $path_to.'/';
			} else {
				$path_to = '/';
			}
			$filetmp = SQ_TEMP_PATH.'/export-'.$export_date.$path_to.$value['name'];
			
			if ($structure && $value['root'] == 0) {
				// We only need the root files and folders here, because Archive Tar handles the structure
				$filelist[] = '.'.$path_to.$value['name'];
			} else if (!$structure) {
				// Here, if there is no structure, we need all files
				$filelist[] = './'.$value['name'];
			}
			// Open to the tmp directory
			if (isset($value['path']) && !empty($value['path']) && !$value['dir']) {
				$to_tmp = copy($value['path'], $filetmp);
				if (!$to_tmp) {
					trigger_localised_error('CMS0105', E_USER_NOTICE);
					trigger_error($filetmp);
					return FALSE;
				}//end if
			} else {
				$mkdir = mkdir($filetmp, 0777);
				if (!$mkdir) {
					trigger_localised_error('CMS0105', E_USER_NOTICE);
					trigger_error($filetmp);
					return FALSE;
				}//end if
			}//end if
		}//end foreach

		// Get the current dir
		$cwd = getcwd();

		// Change to the TEMP dir
		chdir(SQ_TEMP_PATH.'/export-'.$export_date);

		require_once 'Archive/Tar.php';

		$tar_ball = new Archive_Tar($save_location.'/'.$save_name, 'gz');
		$result = $tar_ball->add($filelist);
		if (PEAR::isError($result)) {
			trigger_localised_error('CMS0105', E_USER_NOTICE);
			return FALSE;
		}

		// Change back
		chdir($cwd);

		// Cleanup
		$cleanup = TRUE;
		$cleanup = $this->_cleanup(SQ_TEMP_PATH.'/export-'.$export_date);
		if (!$cleanup) {
			trigger_localised_error('CMS0106', E_USER_NOTICE);
		}
		return TRUE;

	}//end processExportNode()


	/**
	* Find the path to the current file
	*
	* @param array	$list	The list of the current files
	* @param array	$file	The current file
	* @param string	$path	The current path
	*
	* @return string
	* @access private
	*/
	function _findPath($list, $file, $path='')
	{
		// Error checking
		if (!is_array($list) || empty($list)) {
			return $path;
		}
		if (!is_array($file) || empty($file)) {
			return $path;
		}

		if (!isset($file['root'])) return $path;

		$tmp = $file['root'];

		if (isset($list[$tmp]['name']) && !empty($list[$tmp]['name'])) {
			if (isset($list[$tmp]['dir']) && $list[$tmp]['dir']) {
				$path = '/'.$list[$tmp]['name'].$path;
			}
			$path = $this->_findPath($list, $list[$tmp], $path);
		}

		return $path;

	}//end _findPath()


	/**
	* Get a list of files based on the root node
	*
	* @param array	$root_node	The root node or the location to start searching from
	* @param array	$filetypes	The allowed filetypes to match (blank to match all)
	* @param int	&$depth		A count showing the depth
	*
	* @return array
	* @access private
	*/
	function _getFiles($root_node, $filetypes=Array(), &$depth)
	{
		$files = Array();
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Error checking
		if (empty($root_node)) return $files;
		if (!is_array($filetypes)) return $files;

		$child_links = $am->getLinks($root_node, SQ_SC_LINK_BACKEND_NAV, $filetypes, FALSE);
		$files += $this->_processFiles($child_links, $filetypes, $depth);

		return $files;

	}//end _getFiles()


	/**
	* Process the list of files based on the links returned
	*
	* @param array	$links		An array of links of the files
	* @param array	$filetypes	The allowed filetypes to match (blank to match all)
	* @param int	&$depth		A count showing the depth
	*
	* @return array
	* @access private
	*/
	function _processFiles($links, $filetypes, &$depth)
	{
		$files = Array();
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Error checking
		if (!is_array($links) || empty($links)) {
			return $files;
		}
		if (!is_array($filetypes)) return $files;

		$parent = $depth;
		foreach ($links as $link) {
			if (!isset($link['minorid'])) return $files;

			$asset = $am->getAsset($link['minorid']);
			$asset_webpaths = $asset->getWebPaths();
			$asset_webpath = $asset_webpaths[0];

			if (!is_null($asset)) {
				if (is_a($asset, 'folder')) {
					$depth++;
					$files[$depth]['name']		= $asset->name;
					$files[$depth]['path']		= $asset->data_path.'/'.$asset->name;
					$files[$depth]['root']		= $parent;
					$files[$depth]['dir']		= 1;
					$files[$depth]['webpath'] 	= $asset_webpath;
					$files += $this->_getFiles($asset->id, $filetypes, $depth);
				} else {
					if (!empty($filetypes)) {
						foreach ($filetypes as $type) {
							if ($type != 'folder') {
								if (is_a($asset, $type)) {
									$depth++;
									$files[$depth]['name']		= $asset->name;
									$files[$depth]['path']		= $asset->data_path.'/'.$asset->name;
									$files[$depth]['root']		= $parent;
									$files[$depth]['dir']		= 0;
									$files[$depth]['webpath']   = $asset_webpath;
								}//end if
							}//end if
						}//end foreach
					} else {
						$depth++;
						$files[$depth]['name']		= $asset->name;
						$files[$depth]['path']		= $asset->data_path.'/'.$asset->name;
						$files[$depth]['root']		= $parent;
						$files[$depth]['dir']		= 0;
						$files[$depth]['webpath']   = $asset_webpath;
					}//end if
				}//end if
			}//end if
		}//end foreach

		return $files;

	}//end _processFiles()


	/**
	* Cleanup
	*
	* @param string	$base	The base location from where to cleanup from
	*
	* @return boolean
	* @access private
	*/
	function _cleanup($base)
	{
		// Error checking
		if (!is_string($base) || empty($base)) {
			return FALSE;
		}

		$safe_to_remove = FALSE;

		if (is_dir($base)) {
			$base_dir = opendir($base);
			while (FALSE !== ($file = readdir($base_dir))) {
				if ($file == '.' || $file == '..') {
					continue;
				}
				$current_file = $base.'/'.$file;
				if (is_dir($current_file)) {
					$next = $this->_cleanup($current_file);
					if (!$next) {
						trigger_localised_error('CMS0106', E_USER_NOTICE);
						return FALSE;
					}
				} else {
					$delete_file = unlink($current_file);
					if (!$delete_file) {
						trigger_localised_error('CMS0106', E_USER_NOTICE);
						return FALSE;
					}
				}
			}
			closedir($base_dir);

			// Check if empty
			$check = opendir($base);
			$files = Array();
			while (FALSE !== ($filename = readdir($check))) {
				$files[] = $filename;
			}
			sort($files);

			if ((count($files) == 2) && isset($files[1]) && $files[0] == '.' && $files[1] == '..') {
				$safe_to_remove = TRUE;
			}
			closedir($check);

			if ($safe_to_remove) {
				if (!rmdir($base)) return FALSE;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}//end if

		return TRUE;

	}//end _cleanup()


}//end class

?>