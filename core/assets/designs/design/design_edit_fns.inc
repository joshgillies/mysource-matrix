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
* $Id: design_edit_fns.inc,v 1.143 2013/08/06 04:45:17 cupreti Exp $
*
*/

require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';
require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Design_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.143 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	public function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for the parse file uploading
	*
	* @param Design				$asset		the asset to which we belong
	* @param Backend_Outputter	$o			the backend outputter object
	* @param string				$prefix		prefix for the form element
	* @param string     		$onclick_js	Javascript code for the Commit button's click event
	*
	* @return boolean
	* @access public
	*/
	public function paintParseFile(Design $asset, Backend_Outputter $o, $prefix, &$onclick_js='')
	{
		if ($asset->writeAccess('parsing')) {
			file_upload($prefix.'_parse_file', 'id="'.$prefix.'_parse_file'.'" onChange="read_uploaded_file()"');
			hidden_field($prefix.'_parse_file_status', 'valid', 'id="'.$prefix.'_parse_file_status"');
			?>
			<script type="text/javascript">

				/*
				* onChange event function for Upload file button
				* Check the upload file status and update the hidden field "parse_file_status" value
				*/
				function read_uploaded_file()
				{
					var parse_file_field = document.getElementById("<?php echo $prefix; ?>_parse_file");
					var parse_file = parse_file_field.files ? parse_file_field.files[0] : parse_file_field.value;
					if (parse_file) {
						if (window.FileReader) {
							document.getElementById("<?php echo $prefix; ?>_parse_file_status").value = 'loading';
							var reader = new FileReader();
							reader.readAsText(parse_file, "UTF-8");
							reader.onload = function(e) {
								var parse_file_status = document.getElementById("<?php echo $prefix; ?>_parse_file_status");
								if (e.target.result.toLowerCase().indexOf("<mysource_area ") == '-1') {
									parse_file_status.value = 'invalid';
								} else {
									parse_file_status.value = 'valid';
								}
							}

							reader.onerror = function (e) {
								var parse_file_status = document.getElementById("<?php echo $prefix; ?>_parse_file_status");
								parse_file_status.value = 'unknown';
							}
						} else {
							// Without FileReader object support
							var parse_file_status = document.getElementById("<?php echo $prefix; ?>_parse_file_status");
							parse_file_status.value = 'unknown';
						}
					}//end if

				}//end read_uploaded_file()

				/*
				* onClick event function for Commit button
				* Check the hidden field "parse_file_status" value updated by read_uploaded_file() to validate the parse file
				*/
				function validate_parse_file()
				{
					var file_status = document.getElementById("<?php echo $prefix; ?>_parse_file_status").value;
					if (file_status == 'loading') {
						alert("<?php echo translate('The parse file is being read to verify the content. Please press \'Ok\' and then commit again.'); ?>");

						return false;
					} else if (file_status == 'unknown') {
						return confirm("<?php echo translate('Cannot read the parse file to verify its content. Continue processing the parse file anyway?'); ?>");

					} else if (file_status == 'invalid') {
						return confirm("<?php echo translate('The parse file does not contain any Design Area tags. Continue processing the parse file?'); ?>");

					}
					return true;

				}//end validate_parse_file()

			</script>
			<?php
			// The "onclick" event js code for the backend commit button
			if (strtolower(get_class($asset)) == 'design') {
				// We need to validate the content for "Design" asset only
				$onclick_js .= 'if (validate_parse_file()) { if (submit_form) { submit_form(this.form); } else { this.form.submit(); this.disabled = \'disabled\'; }}';
			} else {
				$onclick_js .= 'if (submit_form) { submit_form(this.form); } else { this.form.submit(); this.disabled = \'disabled\'; }';
			}
			return TRUE;
		}

		return FALSE;

	}//end paintParseFile()


	/**
	* Processes the interface for the parse file uploading
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processParseFile(Design $asset, Backend_Outputter $o, $prefix)
	{

		// make sure a file was uploaded before processing it
		$upload_info = get_file_upload_info($prefix.'_parse_file');
		if (empty($upload_info)) return FALSE;

		$parse_file  = $asset->data_path.'/parse.txt';
		$changes     = FALSE;
		$new_version = FALSE;

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		$info = get_file_upload_info($prefix.'_parse_file');
		if(!isset($info['tmp_name'])) {
		    trigger_error('Design parse file is not found', E_USER_WARNING);
		    return FALSE;
		}
		$parse_file_content = file_get_contents($info['tmp_name']);
		if(empty($parse_file_content)) {
		     trigger_error('Empty design parse file is not allowed', E_USER_WARNING);
		     return FALSE;
		}

		// delete existing parse file if it exists
		if (is_file($parse_file)) {
			$new_version = TRUE;
			if (!unlink($parse_file)) {
				trigger_localised_error('CORE0164', translate('Failed deleting existing parse file'), E_USER_WARNING);
				return FALSE;
			}
		}

		// copy over the uploaded file
		if (commit_file_upload($prefix.'_parse_file', $parse_file, TRUE)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = TRUE;
			$asset->_tmp['generate_design'] = TRUE;
			$changes = $this->parseAndProcessFile($asset);

			// if we are overwriting our current parse file, we need to add a new version to the repository
			if ($new_version) {
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '')) {
						trigger_localised_error('CORE0160', translate('Failed committing file version'), E_USER_WARNING);
					}
				}
			} else {
				// attempt to add the parse file to the repository
				if (!$fv->add($asset->data_path_suffix, $parse_file, '')) {
					trigger_localised_error('CORE0157', translate('Failed adding parse file version'), E_USER_WARNING);
				}
			}
		}//end if file uploaded

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
			trigger_localised_error('CORE0158', translate('Failed checking out latest version of parse file'), E_USER_WARNING);
			return FALSE;
		}

		return $changes;

	}//end processParseFile()


	/**
	* Paint the interface for the reparsing the current parse file
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintReparseFile(Design $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$parse_file = $asset->data_path.'/parse.txt';
			if (!is_file($parse_file)) {
				echo translate('Not available - no existing parse file available');

			} else {
				check_box($prefix.'_reparse');
				label(translate('Yes, manually reparse the existing file'), $prefix.'_reparse');

			}
			return TRUE;
		}
		return FALSE;

	}//end paintReparseFile()


	/**
	* Processes the interface for the reparsing the current parse file
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processReparseFile(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_reparse'])) {
			if ($this->parseAndProcessFile($asset)) {
				$asset->_tmp['update_customisations'] = TRUE;
				$asset->_tmp['generate_design'] = TRUE;
				return TRUE;
			}
		}
		return FALSE;

	}//end processReparseFile()


	/**
	* Paint the interface for editing the parse file
	*
	* @param Design				$asset		the asset to which we belong
	* @param Backend_Outputter	$o			the backend outputter object
	* @param string				$prefix		prefix for the form element
	* @param string     		$onclick_js	Javascript code for the Commit button's click event
	*
	* @return boolean
	* @access public
	*/
	public function paintEditParseFile(Design $asset, Backend_Outputter $o, $prefix, &$onclick_js='')
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (SQ_ROLLBACK_VIEW) {
			// get an older version of the parse file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $asset->data_path_suffix.'/parse.txt';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = @$fv->_checkOutCheck($rep_file, NULL, $then);
			$parse_file = $info['source_file'];
		} else {
			$parse_file = $asset->data_path.'/parse.txt';
		}
		if (!is_file($parse_file)) {
			echo translate('You need to upload a parse file before you can edit it');

			return TRUE;
		}

		$str = file_to_string($parse_file);

		if ($asset->writeAccess('parsing')) {
			text_area($prefix.'_new_parse_file', str_replace("\t", '    ', $str), 5000, 20, 0, 'style="" class="sq-parse-file-textarea"');
			?>
			<script type="text/javascript">
				function validate_parse_file()
				{	var parse_file_field = document.getElementById("<?php echo $prefix; ?>_new_parse_file");
					if (parse_file_field.value.toLowerCase().indexOf("<mysource_area ") == '-1') {
						return confirm("<?php echo translate('The parse file does not contain any Design Area tags. Continue processing the parse file?'); ?>");

					}
					return true;
				}
			</script>
			<?php
			// We need to validate the content for "Design" asset only
			if (strtolower(get_class($asset)) == 'design') {
				// The "onclick" event js code for the backend commit button
				$onclick_js .= 'if (validate_parse_file()) { if (submit_form) { submit_form(this.form); } else { this.form.submit(); this.disabled = \'disabled\'; }}';
			} else {
				$onclick_js .= 'if (submit_form) { submit_form(this.form); } else { this.form.submit(); this.disabled = \'disabled\'; }';
			}
			return TRUE;

		} else {
			// FR #2381 - Colour coding the parse file
			$str = htmlspecialchars($str, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			$plain_str = $str;

			// Mysource codes
			$my_style	= 'style="color:#FF0517;font-weight:bold;"';
			$mysrc_search	= Array(
								'/&lt;MySource_(.*?(&gt;))/ims',
								'/&lt;\/MySource_(.*?(&gt;))/ims',
							  );
			$mysrc_replace	= Array(
								'<span '.$my_style.'>&lt;MySource_${1}</span>',
								'<span '.$my_style.'>&lt;/MySource_${1}</span>',
							  );
			$str = preg_replace($mysrc_search, $mysrc_replace, $str);

			// CSS codes
			function wrap_css($matches)
			{
				$css_style	= 'style="color:#6A287E;font-style:italic;"';
				$return_string  = '';
				$return_string .= '<span '.$css_style.'>&lt;style'.$matches[1];
				$white_space = Array("\r\n", "\n", "\r");
				$return_string .= str_replace($white_space, "</span>\n<span ".$css_style.'>', $matches[3]);
				$return_string .= '</span>';
				return $return_string;
			}
			$css_search		= Array(
								'/&lt;style(.*?(&gt;))(.*?(&lt;\/style&gt;))/ims',
							  );
			$str = preg_replace_callback($css_search, "wrap_css", $str);

			// HTML codes
			$html_style	= 'style="color:#0000A0;"';
			$html_keywords = '!DOCTYPE|a|html|head|title|meta|link|body|script|div|span|h1|h2|h3|h4|h5|h6|ul|li|ol|table|tr|td|th|p|br|strong|pre';
			$html_search	= Array(
								'/&lt;('.$html_keywords.')&gt;/i',
								'/&lt;('.$html_keywords.')(.*?(\/&gt;))/ims',
								'/&lt;('.$html_keywords.')(.*?(&gt;))/ims',
								'/&lt;\/('.$html_keywords.')&gt;/ims',
							  );
			$html_replace	= Array(
								'<span '.$html_style.'>&lt;${1}&gt;</span>',
								'<span '.$html_style.'>&lt;${1}${2}</span>',
								'<span '.$html_style.'>&lt;${1}${2}</span>',
								'<span '.$html_style.'>&lt;/${1}&gt;</span>',
							  );
			$str = preg_replace($html_search, $html_replace, $str);

			// Style comments
			$comment_types = Array(
							'/(&lt;!--)(?!@@)(.*?(?!@@)(--&gt;))/ims' => 'color:#347C17;', // HTML comment
							'/(&lt;!--)(@@.*?@@(--&gt;))/ims' => 'color:#B19420;', // Matrix HTML comment
							'/(\/\*)(?!@@)(.*?(?!@@)(\*\/))/ms' => 'color:#347C17;', // JS/CSS comment
							'/(\/\*)(@@.*?@@(\*\/))/ms' => 'color:#B19420;', // Matrix JS/CSS comment
						);
			foreach($comment_types as $cmmt_search => $style) {
				$str = preg_replace_callback(
								$cmmt_search,
								function($matches) use ($style) {
									$cmmt_style	= 'style="'.$style.'"';
									$return_string  = '';
									$return_string .= '<span '.$cmmt_style.'>'.$matches[1];
									$white_space = Array("\r\n", "\n", "\r");
									$return_string .= str_replace($white_space, "</span>\n<span ".$cmmt_style.'>', $matches[2]);
									$return_string .= '</span>';
									return $return_string;
								},
								$str
						);
			}//end foreach

			// Line Numbering and cleanup
			$pre_style	= 'style=""';
			$line_style	= 'style=""';
			$lines = explode("\n", $str);
			$number_of_lines = count($lines);
			$padding = strlen($number_of_lines);
			foreach ($lines as $line_number => $line) {
				// Format each line
				$lines[$line_number] = '<li '.$line_style.' value="'.($line_number+1).'">&nbsp;&nbsp;'.$line.'</li>';
			}
			$formatted = implode('', $lines);
			echo '<pre ', $pre_style, ' class="sq-parse-file-preview"><ol>', $formatted, '</ol></pre>';
		}
		return FALSE;

	}//end paintEditParseFile()


	/**
	* Processes the interface for the editing the parse file
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processEditParseFile(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_parse_file'])) {

			$parse_file  = $asset->data_path.'/parse.txt';
			$new_file    = $_POST[$prefix.'_new_parse_file'];

			if (!is_file($parse_file)) return FALSE;

			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			if(empty($new_file)) {
			    trigger_error('Empty design parse file is not allowed', E_USER_WARNING);
			    return;
			}

			// copy over the uploaded file
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (string_to_file($new_file, $parse_file)) {
				// tell the asset to update the customisation at the end of the interface processing
				$asset->_tmp['update_customisations'] = TRUE;
				$asset->_tmp['generate_design'] = TRUE;
				$changes = $this->parseAndProcessFile($asset);
				if (!$changes) {
					return FALSE;
				}

				// add a new version to the repository
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '')) {
						trigger_localised_error('CORE0160', translate('Failed committing file version'), E_USER_WARNING);
					}
				} else {
					$changes = TRUE;
				}

			} else {
				trigger_localised_error('CORE0167', translate('Failed writing parse file, trying to recover previous version from repository'), E_USER_WARNING);
			}

			// make sure we have the latest version of our file
			if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
				trigger_localised_error('CORE0158', translate('Failed checking out latest version of parse file'), E_USER_WARNING);
				return FALSE;
			}

			if ($changes) {
				return $this->parseAndProcessFile($asset);
			}
		}//end if new parse file submitted

		return FALSE;

	}//end processEditParseFile()


	/**
	* Paint New Files
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNewFiles(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		for ($i = 0; $i < 2; $i++) {
			file_upload($prefix.'_assoc_file_'.$i);
			echo "<br />\n";
		}

	}//end paintNewFiles()


	/**
	* Process New Files
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNewFiles(Design $asset, Backend_Outputter $o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$changes = FALSE;

		for ($i = 0; $i < 2; $i++) {
			$info = get_file_upload_info($prefix.'_assoc_file_'.$i);
			if (empty($info)) continue;

			// if this is a tar archive, extract it
			if (preg_match('/\\.tar\\.gz$/', $info['name'])
				|| preg_match('/\\.tgz$/', $info['name'])
				|| preg_match('/\\.tar$/', $info['name']) ) {

				require_once 'Archive/Tar.php';
				$tar_ball = new Archive_Tar($info['tmp_name']);
				if (($contents = $tar_ball->listContent()) != 0) {

					// Basically we want to make sure that all the files in the
					// tar ball end up in a single directory, so we have to mess about with the extracting a bit
					$extract_list = Array();
					foreach ($contents as $entry) {
						if ($entry['typeflag'] != '0' && $entry['typeflag'] != '') {
							continue;
						}

						$k = dirname($entry['filename']);
						if (!isset($extract_list[$k])) {
							$extract_list[$k] = Array();
						}
						$extract_list[$k][] = $entry['filename'];

					}//end foreach

					foreach ($extract_list as $remove_path => $files) {
						// extract the files from the tar archive to a temporary directory
						$files_dir = $asset->data_path.'/.temp_files';
						if (!create_directory($files_dir)) return FALSE;
						if (!clear_directory($files_dir)) return FALSE;

						$result = $tar_ball->extractList($files, $files_dir, $remove_path);
						if (!$result) {
							trigger_error('Failed to extract tar archive', E_USER_WARNING);
							break;
						} else {
							$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
							$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);

							foreach ($files as $filename) {
								$filename = preg_replace('|^'.$remove_path.'/|', '', $filename);
								$temp_path = $files_dir.'/'.$filename;
								$temp_info = Array('name' => $filename, 'tmp_name' => $temp_path);
								$temp_info['non_uploaded_file'] = TRUE;
								if ($this->_processUploadedFile($asset, $temp_info)) {
									$changes = TRUE;
								}
							}
						}

						delete_directory($files_dir);

					}//end foreach

				}//end if

			// just create the file asset
			} else {

				if ($this->_processUploadedFile($asset, $info)) {
					$changes = TRUE;
				}

			}//end if tar archive

		}//end for

		return $changes;

	}//end processNewFiles()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	*
	* @param Design	$asset	the asset to link the file to
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	public function _processUploadedFile(Design $asset, $info=Array())
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'file', FALSE);

		foreach ($existing as $link) {
			$existing_ids[$link['minorid']] = $link['linkid'];
		}

		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));

		// must web-path-ify the new file name to compare, as this is eventually
		// it's what the file will be named
		$new_file_name = make_valid_web_paths(Array($info['name']));
		$new_file_name = array_shift($new_file_name);

		$existing_fileid = 0;
		foreach ($existing_info as $asset_id => $asset_info) {
			// if the name is the same, delete the link
			if ($asset_info['name'] == $new_file_name) {
				$existing_fileid = $asset_id;
				break;
			}
		}

		if (!$existing_fileid) {
			// Check the filetype of the file being uploaded and create the
			// appropriate asset type
			switch (get_file_type($info['name'])) {
				case 'css' :
						$new_asset_type = 'css_file';
				break;
			    	case 'js' :
						$new_asset_type = 'js_file';
				break;
				case 'gif' :
				case 'jpg' :
				case 'jpeg' :
				case 'png' :
						$new_asset_type = 'image';
				break;
				default :
						$new_asset_type = 'file';
				break;
			}//end switch

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($new_asset_type);
			$new_file = new $new_asset_type();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $info['name']);
			$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);

			return $new_file->create($design_link, $info);

		} else {

			// we already have a design file with the same name, so just upload over the top of it
			$new_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_fileid);

			$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($new_file->id, 'attributes');
			$edit_fns = $new_file->getEditFns();
			$o = NULL;
			$success = $edit_fns->processFileUpload($new_file, $o, $new_file->getPrefix(), $info);

			if ($lock_status === 1) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_file->id, 'attributes');
			}

			return $success;
		}

	}//end _processUploadedFile()


	/**
	* Paint New Customisation
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNewCustomisation(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('all')) {
			$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');
			echo sprintf(translate('You must acquire all locks on this %s to be able to create a new customisation'), strtolower($type_name));

			return FALSE;
		}

		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.' - Customisation', 40);
		return TRUE;

	}//end paintNewCustomisation()


	/**
	* Process New Customisation
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNewCustomisation(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_new_customisation']) || !($new_name = trim($_POST[$prefix.'_new_customisation_name']))) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// First let's duplicate ourselves, ignoring dependants and the directory with the files
		$map = Array();
		// the new link
		$create_link = Array();
		// increase run level to allow setting of components without cloning permissions from parents
		if ($asset->writeAccess('all')) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);
			// This clone will be morphed to customisation later, so it will NOT clone the parse file.
			$asset->_tmp['custom'] = TRUE;
			$clone = $GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions', 'roles'), FALSE, FALSE, $asset->status);
			unset ($asset->_tmp['custom']);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Now let's morph the dupe into a design customisation
		if (!$cloned = $clone->morph('design_customisation')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and link it back to ourselves
		if (!$asset->createLink($cloned, SQ_LINK_TYPE_2, 'customisation', NULL, '1', '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!($cloned->setAttrValue('id_name', $new_name) && $cloned->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach ($da_links as $link) {
			$da = $am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			if (!$cloned->createLink($da, SQ_LINK_TYPE_3, $link['value'], NULL, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $cloned->id, TRUE);
		if (!$cloned->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$minor = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$cloned->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0162', translate('Failed creating file link to customisation'), E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// link up all the css files from this design to the customisation
		$css_link_ids = Array();
		$css_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'design_css', FALSE);
		foreach ($css_links as $link) {
			$minor = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$cloned->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0161', translate('Failed creating css file link to customisation'), E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// we skipped this step in cloneComponentsAdditional(), now we since got all design areas linked, it's time to generate a design file
		$cloned->generateDesignFile();

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($cloned->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processNewCustomisation()


	/**
	* Paint current customisation
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentCustomisations(Design $asset, Backend_Outputter $o, $prefix)
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
		if ($customisation_links) {
			$am = $GLOBALS['SQ_SYSTEM']->am;
			foreach ($customisation_links as $link) {
				echo get_asset_tag_line($link['minorid'], 'details').'<br/>';
			}

		} else {
			echo translate('No Customisations Created');

		}

	}//end paintCurrentCustomisations()


	/**
	* Parses the parse file and processes the contents
	*
	* @param Design	$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	public function parseAndProcessFile(Design $asset)
	{
		$parse_file = $asset->data_path.'/parse.txt';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = file_to_string($parse_file);
		$contents = $this->_parseString($asset, $str);
		$res = (!is_null($contents) && $this->_processContents($asset, $contents) && $asset->saveAttributes());
		return $res;

	}//end parseAndProcessFile()


	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param Design	$asset			the asset to which we belong
	* @param string	$file_contents	the contents to parse
	*
	* @return array
	* @access public
	*/
	public function _parseString(Design $asset, $file_contents)
	{
		// convert the head and body tags to lowercase
		$file_contents = preg_replace(	Array('/<head/i', '/<\\/head/i', '/<body/i', '/<\\/body/i'),
										Array('<head', '</head', '<body', '</body'),
										$file_contents);


		// Let's place an print operation after the head tag
		// to allow us to print the default header stuff like
		// style sheets, JS files and meta tag stuff
		if(strpos($file_contents, '</head>') !== FALSE) {
		    $file_contents = str_replace('</head>', "\n<".$this->tag_name."_PRINT id_name='__global__' var='html_header' />\n</head>", $file_contents);
		}

		// Let's place an print operation after the termination body tag
		// to allow us to print the anything that has come to our attention
		// like image rollovers
		if(strpos($file_contents, '</body>') !== FALSE) {
		    $file_contents = str_replace('</body>', "\n<".$this->tag_name."_PRINT id_name='__global__' var='html_footer' />\n</body>", $file_contents);
		}


		return parent::_parseString($file_contents);

	}//end _parseString()


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design	$asset		the asset to which we belong
	* @param array	$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design $asset, Array $contents)
	{
		$id_names = Array();
		// let's check to see if all the design areas have unique names
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') {
				continue;
			}

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['id_name']) || empty($element['attributes']['design_area'])) {
				trigger_localised_error('CORE0143', translate('All design areas need to have an "id_name" and a "design_area"'), E_USER_WARNING);
				return FALSE;
			}//end if

			// validate id name
			if (!preg_match('/^[a-z][a-z0-9_]*$/i', $element['attributes']['id_name']) || $element['attributes']['id_name'] != preg_replace('/__+/', '', $element['attributes']['id_name'])) {
				trigger_localised_error('CORE0181', sprintf('The id_name "%s" contains illegal characters, the id_name must be an alpha character, optionally followed by any number of alphanumeric characters or underscores. NOTE: you cannot have more that one consecutive underscore', $element['attributes']['id_name']), E_USER_WARNING);
				return FALSE;
			}

			$element['attributes']['design_area'] = 'design_area_'.strtolower($element['attributes']['design_area']);

			// if there is a name and it's not the the same type as the current design area
			if (isset($id_names[$element['attributes']['id_name']])) {
				trigger_localised_error('CORE0142', sprintf(translate('All design areas MUST have a unique id_name, "%s" is used at least twice'), $element['attributes']['id_name']), E_USER_WARNING);
				return FALSE;
			}//end if

			$id_names[$element['attributes']['id_name']] = $element['attributes']['design_area'];

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (isset($GLOBALS['SQ_PROCESSED_DESIGN_AREAS'])) {
			// let's remove any design area links that we aren't going to be using any more
			$existing_da_links = $asset->getDesignAreaLink();
			$existing_id_names = Array();
			foreach ($existing_da_links as $link) {
				$existing_id_names[$link['value']] = $link['linkid'];
			}

			$removed_id_names = array_diff(array_keys($existing_id_names), array_keys($GLOBALS['SQ_PROCESSED_DESIGN_AREAS']));

			// now remove any unused design area customisations
			foreach ($removed_id_names as $id_name) {
				if (!$asset->deleteLink($existing_id_names[$id_name])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}//end foreach
		} else {
			$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'] = Array();
		}

		// validate the design areas that we are going to use

		$design_areas = Array();

		// now let's check to see if any existing design areas exists
		// and check if they are the same type as the one in the contents
		foreach ($id_names as $id_name => $design_area) {

			if (!$GLOBALS['SQ_SYSTEM']->am->installed($design_area)) {
				trigger_localised_error('CORE0327', sprintf(translate('Could not parse design file. Design Area "%s" is not installed in the system.'), $design_area), E_USER_WARNING);
				return FALSE;
			}

			$da = $asset->getDesignArea($id_name);

			// if it's a different type, then convert them
			if (!is_null($da) && $da->type() !== $design_area) {
				$link = $asset->getDesignAreaLink($id_name, FALSE);
				if (!$asset->deleteLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				unset($da); // unset so we don't set the value or all ref's to NULL
				$da = NULL; // do this so a new version will be created
			}

			// if it doesn't exist, create it, and acquire the lock
			if (is_null($da)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da = new $design_area();
				$da_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'value' => $id_name, 'is_dependant' => '1');
				$da->setAttrValue('id_name', $id_name);
				if (!$da->create($da_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($da->id, 'parsing')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			$design_areas[$id_name] = $da;

		}//end foreach


		// Now that we have set all the design areas up we can get on with the processing of the contents array
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') {
				continue;
			}

			$edit_fns = $design_areas[$element['attributes']['id_name']]->getEditFns();
			if (!$edit_fns->_processContents($design_areas[$element['attributes']['id_name']], $element['contents'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$print = !in_array(array_get_index($element['attributes'], 'print', 'yes'), Array('no', 'false', '0'));
			$design_areas[$element['attributes']['id_name']]->setAttrValue('print', $print);
			$cache = in_array(array_get_index($element['attributes'], 'cache', FALSE), Array('yes', 'true', '1'));
			$design_areas[$element['attributes']['id_name']]->setAttrValue('cache', $cache);

			if (!$design_areas[$element['attributes']['id_name']]->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			// we are done with this design area, release the lock
			// otherwise we will get lock by user 0 after a full system cleanup
			// for login form design area and password change form design area
			// this only occurs in CLI mode
			if (SQ_PHP_CLI) {
				$lockInfo = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($design_areas[$element['attributes']['id_name']]->id, 'parsing');
				if (!empty($lockInfo['links']) && $lockInfo['links']['userid'] == '0') {
					@$GLOBALS['SQ_SYSTEM']->am->releaseLock($design_areas[$element['attributes']['id_name']]->id, 'parsing');
				}
			}

		}//end foreach

		// now just set the contents and we are away...
		if ($asset->setAttrValue('contents', $contents)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

	}//end _processContents()


	/**
	* Paints the facility to upload and import files into the design
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintImport(Design $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('parsing');
		$o->openField(translate('Import File'));

			if ($write_access) {
				file_upload($prefix.'_import_file');
			}
			$o->note(translate('Select a file to import as a design file.'));

		$o->closeField();
		$o->openField(translate('Existing Asset'));

			if ($write_access) {
				asset_finder($prefix.'_import_asset', 0, Array('file' => 'I'));
			}
			$o->note(translate('Select an existing file asset to import to this design'));

		$o->closeField();

		return TRUE;

	}//end paintImport()


	/**
	* Processes uploaded files, and creates required assets and links to generate this design
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processImport(Design $asset, Backend_Outputter $o, $prefix)
	{
		$tar_upload = get_file_upload_info($prefix.'_import_file');
		if ($tar_upload === FALSE) {
			trigger_error(translate('Error uploading Design Archive'), E_USER_WARNING);

		} else {
			if (!empty($tar_upload)) {
				$import_dir = $asset->data_path.'/temp/';
				if (!is_dir($import_dir)) {
					create_directory($import_dir);
				}
				commit_file_upload($prefix.'_import_file', $import_dir, TRUE, 0, TRUE, Array('gz', 'tar', 'tgz'));

				$filename = $tar_upload['name'];
				$import_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'export_file');
				$temp_info = Array('name' => $filename, 'tmp_name' => $import_dir.'/'.$filename, 'non_uploaded_file' => TRUE);

				$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
				$new_file = new File();
				$new_file->_tmp['uploading_file'] = TRUE;
				$new_file->setAttrValue('name', $filename);

				if (!$new_file->create($import_link, $temp_info)) {
					trigger_error('Failed to import File '.$filename, E_USER_WARNING);
				}
				delete_directory($import_dir);
			}
		}

		$selected_asset = $_POST[$prefix.'_import_asset']['assetid'];
		if ($selected_asset != '0') {
			$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_asset);
			$asset->createLink($file, SQ_LINK_TYPE_1, 'export_file');
		}

		return TRUE;

	}//end processImport()


	/**
	* Paints the interface for exporting this design
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintExport(Design $asset, Backend_Outputter $o, $prefix)
	{
		$o->sectionNote(translate('Customisations will not be exported. They will need to be recreated when this design is imported and applied.'));

		$write_access = $asset->writeAccess('parsing');
		$o->openField(translate('Export File Name'));

			if ($write_access) {
				text_box($prefix.'_export_filename', '');
			}
			$o->note(translate('Enter a file name to export the current design to. The new file will be suffixed with \'.tar.gz\' automatically.'));

		$o->closeField();

		return TRUE;

	}//end paintExport()


	/**
	* Processes the options and exports the assets required to recreate this design
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processExport(Design $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('parsing');

		if ($write_access) {
			if ($_POST[$prefix.'_export_filename'] != '') {
				$new_file_name = $_POST[$prefix.'_export_filename'];
				if (strpos($new_file_name, '.tar.gz') === FALSE) {
					$new_file_name .= '.tar.gz';
				}

				$files_dir = $asset->data_path.'/.temp_files';
				if (!create_directory($files_dir)) return FALSE;
				if (!clear_directory($files_dir)) return FALSE;

				// Get the current links
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);

				$files_to_copy = Array();
				foreach ($links as $link_data) {
					if ($link_data['value'] == 'export_file') continue;
					$child_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid']);
					$current_file_info = $child_asset->getExistingFile();
					$files_to_copy[$current_file_info['filename']] = $current_file_info['path'];
				}
				$archive_files = Array();
				foreach ($files_to_copy as $filename => $path) {
					copy($path, $files_dir.'/'.$filename);
					$archive_files[] = $files_dir.'/'.$filename;
				}

				require_once 'Archive/Tar.php';

				$tar_object = new Archive_Tar($files_dir.'/associated_files.tar.gz', TRUE);
				$tar_object->createModify($archive_files, '', $files_dir);

				foreach ($archive_files as $remove) {
					unlink($remove);
				}

				$archive_files = Array();

				// Should have only the associated files tar in the directory, now we just need to add the parse file
				if (!file_exists($asset->data_path.'/parse.txt')) {
					trigger_error('No Parse File to Export', E_USER_WARNING);
					return FALSE;
				}

				copy($asset->data_path.'/parse.txt', $files_dir.'/parse.html');

				$archive_files[] = $files_dir.'/parse.html';
				$archive_files[] = $files_dir.'/associated_files.tar.gz';

				$tar_object = new Archive_Tar($files_dir.'/'.$new_file_name, TRUE);
				$tar_object->createModify($archive_files, '', $files_dir);

				$temp_path = $files_dir.'/'.$new_file_name;
				$temp_info = Array('name' => $new_file_name, 'tmp_name' => $temp_path);
				$temp_info['non_uploaded_file'] = TRUE;

				$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');

				$new_file = new File();
				$new_file->_tmp['uploading_file'] = TRUE;
				$new_file->setAttrValue('name', $new_file_name);
				$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'export_file', 'sort_order' => 1, 'is_dependant' => 1);

				$new_file->create($design_link, $temp_info);

				delete_directory($files_dir);
			}//end if not empty filename
		}//end if write_access

		return $write_access;

	}//end processExport()


	/**
	* Paints files that are currently underneath this file
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentFiles(Design $asset, Backend_Outputter $o, $prefix)
	{
		$o->sectionNote(translate('Applying a design will remove all customisations and associated files from the current design.'));

		$write_access = $asset->writeAccess('parsing');
		?>
			<table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" style="width: 200px;"><?php echo translate('Filename'); ?></td>

					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-header" style="width: 75px;"><?php echo translate('Download'); ?></td>

						<td class="sq-backend-table-header" style="width: 75px;"><?php echo translate('Apply'); ?></td>

						<?php
					}
					?>
				</tr>
				<?php
					$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'file', TRUE, 'major', 'export_file');
					$file_ids = Array();
					foreach ($file_links as $data) {
						$file_ids[] = $data['minorid'];
					}
					$file_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($file_ids);
					foreach ($file_info as $id => $data) {
						$file_name = $data['name'];
						?>
						<tr>
							<td><?php echo $file_name; ?></td>
							<?php
								if ($write_access) {
								?>
									<td><a href="./?a=<?php echo $id; ?>"><?php echo translate('Download'); ?></a></td>

									<td>
										<?php radio_button($prefix.'_apply', $id);?>
									</td>
									<?php
								}
								?>
						</tr>
						<?php
					}
				?>
				<?php
					if ($write_access) {
						?>
						<tr><td colspan="2"><?php echo translate('No Change'); ?></td><td><?php radio_button($prefix.'_apply', '', TRUE);?></td></tr>

						<?php
					}
				?>
			</table>
		<?php
		return TRUE;

	}//end paintCurrentFiles()


	/**
	* Processes the deletion etc of current files
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCurrentFiles(Design $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('parsing');
		if (!$write_access) return FALSE;

		$file_to_apply = $_POST[$prefix.'_apply'];
		if ($file_to_apply == '') return TRUE;
		$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($file_to_apply);
		$file_info = $file->getExistingFile();

		require_once 'Archive/Tar.php';
		$tar_ball = new Archive_Tar($file_info['path']);

		if (($contents = $tar_ball->listContent()) != 0) {
			// Tar ball is fine.
			$extract_list = Array();
			foreach ($contents as $data) {
				$extract_list[] = $data['filename'];
			}

			$valid_parse_names = Array(
									'parse.txt',
									'parse.html',
									'parse.htm',
									'index.htm',
									'index.html',
								 );

			$parse_file_found = FALSE;
			$parse_file = '';
			foreach ($valid_parse_names as $name) {
				if (in_array($name, $extract_list)) {
					$parse_file = $name;
					$parse_file_found = TRUE;
					break;
				}
			}

			if (!$parse_file_found) {
				trigger_error(translate('No Valid Parse File Found in Archive'), E_USER_WARNING);

				return FALSE;
			}

			$files_dir = $asset->data_path.'/.temp_files';
			if (!create_directory($files_dir)) return FALSE;
			if (!clear_directory($files_dir)) return FALSE;

			$result = $tar_ball->extractList($extract_list, $files_dir);

			// Add and link up the parse file
			$current_parse_file  = $asset->data_path.'/parse.txt';
			$changes     = FALSE;
			$new_version = FALSE;

			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			// delete existing parse file if it exists
			if (is_file($current_parse_file)) {
				$new_version = TRUE;
				if (!unlink($current_parse_file)) {
					trigger_localised_error('CORE0164', translate('Failed deleting existing parse file'), E_USER_WARNING);
					return FALSE;
				}
			}

			if (copy($files_dir.'/'.$parse_file, $current_parse_file)) {

				$asset->_tmp['update_customisations'] = TRUE;
				$asset->_tmp['generate_design'] = TRUE;
				$changes = $this->parseAndProcessFile($asset);

				// if we are overwriting our current parse file, we need to add a new version to the repository
				if ($new_version) {
					$file_status = $fv->upToDate($parse_file);
					if (FUDGE_FV_MODIFIED & $file_status) {
						if (!$fv->commit($current_parse_file, '')) {
							trigger_localised_error('CORE0160', translate('Failed committing file version'), E_USER_WARNING);
						}
					}
				} else {
					// attempt to add the parse file to the repository
					if (!$fv->add($asset->data_path_suffix, $current_parse_file, '')) {
						trigger_localised_error('CORE0157', translate('Failed adding parse file version'), E_USER_WARNING);
					}
				}

				// Remove Current Links
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, '');
				$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				foreach ($links as $data) {
					if ($data['value'] != 'export_file') {
						// Move it to trash
						$trashed = $GLOBALS['SQ_SYSTEM']->am->getAsset($data['minorid']);
						$trashed->saveWebPaths(Array());
						$GLOBALS['SQ_SYSTEM']->am->trashAsset($trashed->id);
					}
				}

				$assoc_tar = '';
				if (in_array('associated_files.tar.gz', $extract_list)) {
					$assoc_tar = 'associated_files.tar.gz';
				}
				if (in_array('associated-files.tar.gz', $extract_list)) {
					$assoc_tar = 'associated-files.tar.gz';
				}
				if ($assoc_tar != '') {
					// Got associated files, now get them linked up
					create_directory($files_dir.'/extracted/');
					require_once 'Archive/Tar.php';
					$nested_tar_ball = new Archive_Tar($files_dir.'/associated_files.tar.gz');

					$nested_tar_ball->extract($files_dir.'/extracted/');
					$assoc_files = list_files($files_dir.'/extracted/', TRUE);

					foreach ($assoc_files as $full_path) {
						$info = Array('name' => basename($full_path), 'tmp_name' => $full_path);
						$info['non_uploaded_file'] = TRUE;
						if (!$this->_processUploadedFile($asset, $info)) {
							trigger_error(sprintf(translate('Problem Uploading Associated File: %s'), basename($full_path)), E_USER_WARNING);

						}
					}
				}

				delete_directory($files_dir);
			}//end if copy successful
		} else {
			trigger_error(sprintf(translate('Error opening file %s'), $file_info['name']), E_USER_WARNING);

		}
		return $write_access;

	}//end processCurrentFiles()


	/**
	* Paint the usage of this paint layout
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintUsage(Design $asset, Backend_Outputter $o, $prefix)
	{
		$page_size = 20;

		$o->openRaw();

		$tree_tops = $GLOBALS['SQ_SYSTEM']->am->getAssetidsByLookupValue('design::%', $asset->id, TRUE);

		if (count($tree_tops) > 0) {
			$tree_tops = $this->_getTreeTops($tree_tops);

			$num_pages = ceil(count($tree_tops) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total assets'), ($page_num - 1) * $page_size + 1, min(count($tree_tops), $page_num * $page_size), count($tree_tops));



			$tree_tops = array_slice($tree_tops, ($page_num - 1) * $page_size, $page_size);

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('Go to the first page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', 1)">'.$links['first'].'</a>';

				$links['previous'] = '<a title="'.translate('Go to the previous page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';

			} else {
				$links['first'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['first'].'</span>';

				$links['previous'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['previous'].'</span>';

			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('Go to the last page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.$num_pages.')">'.$links['last'].'</a>';

				$links['next'] = '<a title="'.translate('Go to the next page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num + 1).')">'.$links['next'].'</a>';

			} else {
				$links['last'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['last'].'</span>';

				$links['next'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['next'].'</span>';

			}

			hidden_field($prefix.'_usage_page', $page_num); ?>
			<table class="sq-backend-table">
			<thead>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-header-header sq-pagination-links sq-no-side-border" style="text-align: left;"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></th>
				<th class="sq-backend-table-header-header sq-no-side-border" style="text-align: right;"><?php echo $asset_count_tag; ?></span></th>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell" colspan="2"><?php echo translate('Asset'); ?></th>

			</tr>
			</thead>
			<tbody>
			<?php

			foreach ($tree_tops as &$tree_info) {
				?><tr class="sq-backend-table-row">
					<td class="sq-backend-table-cell" colspan="2">
					<?php

					$tag_line = get_asset_tag_line($tree_info['assetid'], 'details');

					$child_count_mult = $tree_info['child_count'];
					if ($child_count_mult > 1) $child_count_mult = 'm';
					echo translate('asset_and_child_count_'.$child_count_mult, $tag_line, $tree_info['child_count']);
					?>
				</tr>
				<?php
			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

			$o->note(translate('This screen shows which parts of the asset tree this design is used in, by listing the highest-level assets to which it has been applied and the number of their children to which it has cascaded.'));


		} else {
			$o->note(translate('<p>This design is not applied to any assets.</p>'));

		}

		$o->closeRaw();

	}//end paintUsage()


	/**
	* Get the "tree tops" in the supplied list of assets
	*
	* An asset is a "tree top" in a set of assets if none of its ancestors
	* in the asset tree are in the set with it.  Conversely if an asset's parent
	* or grandparent etc is in the set, then that asset is not a tree top.
	*
	* Returns an array indexed by treeid, where each value is another array
	* containing assetid and number of children of that assetid found in
	* the set.
	*
	* @param array	$assetids	The assetids we are analysing
	*
	* @return array
	* @access public
	*/
	public function _getTreeTops(Array $treeids)
	{
		$all_treeids = Array();
		foreach ($treeids as $assetid => $asset_treeids) {
			foreach ($asset_treeids[0] as $asset_treeid) {
				if (($asset_treeid === 'override') || ($asset_treeid === 'not_override')) {
					continue;
				}
				$all_treeids[$asset_treeid] = $assetid;
			}
		}
		uksort($all_treeids, Array($this, '_strlenCmp'));
		$res = Array();
		foreach ($all_treeids as $treeid => $assetid) {
			$treeid_copy = $treeid;
			while (!empty($treeid_copy)) {
				if (isset($res[$treeid_copy])) {
					$res[$treeid_copy]['child_count']++;
					continue(2);
				}
				$treeid_copy = substr($treeid_copy, 0, -SQ_CONF_ASSET_TREE_SIZE);
			}
			$res[$treeid]['assetid'] = $assetid;
			$res[$treeid]['child_count'] = 0;
		}
		ksort($res); // put in treeid order
		return $res;

	}//end _getTreeTops()


	/**
	* Compare the lengths of two strings
	*
	* @param string	$a	First comparee
	* @param string	$b	Second comparee
	*
	* @return boolean
	* @access public
	*/
	public function _strlenCmp($a, $b)
	{
		return strlen($a) > strlen($b);

	}//end _strlenCmp()


	/**
	* Generic fucntion to paint the interface for dropdown classes selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	* @param string					$attribute name of the attribute to paint
	*
	* @return boolean
	* @access public
	*/
	public function _paintSelection(Design $asset, Backend_Outputter $o, $prefix, $attribute)
	{
		$write_access = $asset->writeAccess('attributes');
		$select_options = $asset->attr($attribute);

		if (!empty($select_options)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('Class Name'); ?></td>

					<td class="sq-backend-table-header"><?php echo translate('Short Name'); ?></td>

					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header"><?php echo translate('Delete?'); ?></td><?php

					}
					?>
				</tr>
				<?php
				$i = 0;
				foreach ($select_options as $option_key => $option_value) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
							if ($write_access) {
								text_box($prefix.'_options[key]['.$i.']', $option_key, 20);
							} else {
								echo $option_key;
							}
							?>
						</td>
						<td class="sq-backend-table-cell" align="left">
							<?php
							if ($write_access) {
								text_box($prefix.'_options[val]['.$i.']', $option_value, 20);
							} else {
								echo $option_value;
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_options[del]['.$i.']', $option_key); ?>
							</td>
							<?php
						}
						?>
					</tr>
					<?php
					$i++;
				}//end foreach select_options
				?>
			</table>
			<?php

		} else {
			echo translate('There are currently no select box options');

		}//end if empty

		// now adding two fields for new entry (key => value)
		if ($write_access) {
			$o->openField(translate('New Option'));

				echo '<b>'.translate('Key').'</b> ';

				text_box($prefix.'_new_key','');
				echo ' <b>'.translate('Value').'</b> ';

				text_box($prefix.'_new_val','');
			$o->closeField();
		}

		return TRUE;

	}//end _paintSelection()


	/**
	* Generic function to process the interface for classes dropdown selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	* @param string					$attribute name of the attribute to paint
	*
	* @return boolean
	* @access public
	*/
	public function _processSelection(Design $asset, Backend_Outputter $o, $prefix, $attribute)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$select_options = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$option_changes = $_POST[$prefix.'_options'];
			foreach ($option_changes['key'] as $i => $key) {
				if (isset($option_changes['del'][$i])) continue;
				$select_options[trim($key)] = trim($option_changes['val'][$i]);
			}
		}

		// now get the new submitted values (if any) and add them to array
		if (strlen(trim($_POST[$prefix.'_new_key']))>0 && strlen(trim($_POST[$prefix.'_new_val']))>0) {
			$new_select_key  = trim($_POST[$prefix.'_new_key']);
			$new_select_val  = trim($_POST[$prefix.'_new_val']);
			$select_options[$new_select_key] = $new_select_val;
		}

		// updating asset attribute value
		$asset->setAttrValue($attribute, $select_options);

		return TRUE;

	}//end _processSelection()


	/**
	* Function to paint the interface for WYSIWYG classes dropdown selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function paintWysiwygClasses(Design $asset, Backend_Outputter $o, $prefix)
	{
		$this->_paintSelection($asset, $o, $prefix.'_wysiwyg', 'wysiwyg_classes');
		$o->openField('');
		$o->note(translate('Classes specified in this list will be available in WYSIWYG drop-down list for styles. If the list is empty, css file wil be parsed and all class definitions available.'));

		$o->closeField();

		return TRUE;

	}//end paintWysiwygClasses()


	/**
	* Function to process the interface for WYSIWYG classes dropdown selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function processWysiwygClasses(Design $asset, Backend_Outputter $o, $prefix)
	{
		$this->_processSelection($asset, $o, $prefix.'_wysiwyg', 'wysiwyg_classes');

		return TRUE;

	}//end processWysiwygClasses()


	/**
	* Function to paint the interface for DIV classes dropdown selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function paintDivClasses(Design $asset, Backend_Outputter $o, $prefix)
	{
		$this->_paintSelection($asset, $o, $prefix.'_div', 'div_classes');
		$o->openField('');
		$o->note(translate('Classes specified in this list will be available in DIV properties drop-down list for class. If the list is empty only text field will be used.'));

		$o->closeField();

		return TRUE;

	}//end paintDivClasses()


	/**
	* Function to process the interface for DIV classes dropdown selection box
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function processDivClasses(Design $asset, Backend_Outputter $o, $prefix)
	{
		$this->_processSelection($asset, $o, $prefix.'_div', 'div_classes');

		return TRUE;

	}//end processDivClasses()


	/**
	* Function to paint the File Reference Method field
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function paintFileReferenceMethod(Design $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$attribute = $asset->getAttribute('file_reference_method');
		$attribute->paint($prefix.'_file_reference_method', !$wa);

		return TRUE;

	}//end paintFileReferenceMethod()


	/**
	* Function to process the File Reference Method field
	*
	* @param Design					$asset	the asset whose interface we are painting
	* @param Backend_Outputter		$o		the outputter class
	* @param string					$prefix	prefix for the form elements
	*
	*/
	public function processFileReferenceMethod(Design $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			return FALSE;
		}

		$attribute = $asset->getAttribute('file_reference_method');
		$attribute->process($prefix.'_file_reference_method');
		if ($attribute->processed && $attribute->value != $asset->attr('file_reference_method')) {
			$asset->setAttrValue('file_reference_method', $attribute->value);

			// Update the customisations as well
			$customisations = $asset->getAllCustomisations();
			foreach($customisations as $customisation_id => $type_code) {
				$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($customisation_id);
				$GLOBALS['SQ_SYSTEM']->setRunLevel('SQ_RUN_LEVEL_FORCED');
					$customisation->setAttrValue('file_reference_method', $attribute->value);
					$customisation->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}//end foreach
		}//end if

		return TRUE;

	}//end processFileReferenceMethod()

}//end class

?>
