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
* $Id: image_edit_fns.inc,v 1.66 2013/07/25 23:23:48 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.66 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Edit_Fns extends File_Edit_Fns
{


	/**
	* Prints the interface for uploading a file
	*
	* In this case it gives us the option of launching the image editor too
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(File $asset, Backend_Outputter $o, $prefix)
	{
		parent::paintFileUpload($asset, $o, $prefix);

		if ((!$asset->id) && file_exists(SQ_FUDGE_PATH.'/image_editor/imagej.jar')) {
			// we are in the create screen, so offer the editor too unless we are on Safari Windows
			echo '<p>';
			normal_button($prefix.'_use_editor_button', translate('Use Image Editor'), 'this.parentNode.parentNode.parentNode.style.display=\'none\'; document.getElementById(\''.$prefix.'_hide_editor_button\').parentNode.parentNode.style.display=\'\';', 'style=display:none;');

			?>
			<script type="text/javascript">
				if (!(navigator.userAgent.match(/Windows/i) && navigator.userAgent.match(/Safari/i))) {
					document.getElementById('<?php echo $prefix; ?>_use_editor_button').style.display = 'inline';
				}
			</script>
			<?php
			echo '</p>';
		}

		return TRUE;

	}//end paintFileUpload()


	/**
	* Prints the list of embedded data
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEmbeddedData(&$asset, &$o, $prefix)
	{
		$embedded_data = $asset->attr('embedded_data');

		if (empty($embedded_data)) {
			$o->note('Not found');
			return;
		}

		echo '<table class="sq-backend-table">';
		echo '<tr><th>'.translate('Name').'</th>';

		echo '<th>'.translate('Keyword').'</th>';

		echo '<th>'.translate('Value').'</th></tr>';

		foreach ($embedded_data as $value_set) {
			echo '<tr>';
			echo '<td>';
			echo $value_set['name'];
			echo '</td>';
			echo '<td>';
			echo '%image_embedded_'.$value_set['name'].'%';
			echo '</td>';
			echo '<td>';
			echo is_array($value_set['value']) ? implode('<br />', $value_set['value']) : $value_set['value'];
			echo '</td></tr>';
		}
		echo '</table>';

	}//end paintEmbeddedData()


	/**
	* Processes the interface for uploading a file
	*
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The fields required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	*
	* @param object		&$asset				the file asset
	* @param object		&$o					the outputter class
	* @param string		$prefix				prefix for form elements
	* @param array		$info	info 		about the uploaded file
	* @param boolean	$redirect_in_limbo	redirect when the file name changes in limbo
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(File $asset, $o, $prefix, Array $info=Array(), $redirect_in_limbo=TRUE)
	{
		if (!parent::processFileUpload($asset, $o, $prefix, $info,$redirect_in_limbo)) {
			return FALSE;
		}

		// assetid will not be set if we are creating
		if (!$asset->id) return TRUE;

		$info = $asset->getExistingFile();
		if (!empty($info)) {
			if (!$asset->setAttrValue('width', $info['width'])) {
				return FALSE;
			}
			if (!$asset->setAttrValue('height', $info['height'])) {
				return FALSE;
			}
			if (!$asset->setAttrValue('size', $info['size'])) {
				return FALSE;
			}
		}//end if

		$this->_recreateVarietyImages($asset);


		// extract image file metadata
		$meta = $this->_extractImageMetadata($info['path'], $asset);
		if ($meta !== FALSE) {
			if (!$asset->setAttrValue('embedded_data', $meta)) {
				return FALSE;
			}
		}

		if (!$asset->saveAttributes(TRUE)) {
			trigger_localised_error('CORE0052', translate('Failed saving attributes for file'), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end processFileUpload()



	/**
	* Find out is there are any varieties
	* Returns TRUE if edit interface to be printed
	*
	* @param object	&$asset	the file asset
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access public
	*/
	function hasVarieties(&$asset, $prefix)
	{
		$varieties = $asset->attr('varieties');
		return (!empty($varieties['data']));

	}//end hasVarieties()


	/**
	* Paint the varieties editing section
	* Returns TRUE if edit interface printed
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentVarieties(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$varieties = $asset->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['data'] as $varid => $data) {
				$variety = $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$edit_fns = $variety->getEditFns();

				$o->openSection($variety->name);
					$edit_fns->paintInlineInterface($variety, $o, $prefix.'_'.$variety->id);
					if ($wa) {
						$o->openField(translate('Delete?'));

							check_box($prefix.'_delete[]', $variety->id);
						$o->closeField();
					}
				$o->closeSection();

			}// end foreach
		}// endif

		return $wa;

	}//end paintCurrentVarieties()


	/**
	* Process the varieties editing section
	* Returns TRUE if edit interface printed
	*
	* @param object	&$asset	the file asset
	* @param object	$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentVarieties(&$asset, $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$changes_made = FALSE;

		if (!empty($_POST[$prefix.'_delete']) && is_array($_POST[$prefix.'_delete'])) {
			foreach ($_POST[$prefix.'_delete'] as $assetid) {
				list(, $varid) = explode(':', $assetid, 2);
				if ($asset->deleteVariety($assetid)) {
					$changes_made = TRUE;
				}
			}// end foreach
		}// end if

		$varieties = $asset->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['data'] as $varid => $data) {
				$variety = $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$edit_fns = $variety->getEditFns();
				// If the interface has not been painted, and no file is uploaded for this variety, that means it is new,
				// processNewVariety should already taken care of it
				if (!isset($_FILES[$prefix.'_'.$variety->id]) && !isset($_POST[$prefix.'_'.$variety->id.'_variety_type_posted'])) continue;
				if ($edit_fns->processInlineInterface($variety, $o, $prefix.'_'.$variety->id) && !($asset->status & SQ_SC_STATUS_SAFE_EDITING)) {
					// update the public version with the new file if not in safe edit
					if ($variety->checkFileState($asset->effectiveUnrestricted() && $asset->attr('allow_unrestricted'))) {
						$changes_made = TRUE;
					}
				}
			}// end foreach
		}// end if

		return $changes_made;

	}//end processCurrentVarieties()


	/**
	* Print the new variety section
	* Returns TRUE if edit interface printed
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintNewVariety(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');
		$variety = new Image_Variety($asset->id.':v0');

		$edit_fns = $variety->getEditFns();

		$o->openSection(translate('New Variety'));

			$edit_fns->paintInlineInterface($variety, $o, $prefix.'_new');
		$o->closeSection();

		return $wa;

	}//end paintNewVariety()


	/**
	* Process the "edit image" screen
	* Returns TRUE if edit interface printed
	*
	* @param object	&$asset	the file asset
	* @param object	$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processNewVariety(&$asset, $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_new_name']) || trim($_POST[$prefix.'_new_name']) == '') {
			return TRUE;
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');
		$variety = new Image_Variety($asset->id.':v0');

		$edit_fns = $variety->getEditFns();
		if (!$edit_fns->processInlineInterface($variety, $o, $prefix.'_new')) {
			return FALSE;
		}

		return (bool) $asset->saveVariety($variety);

	}//end processNewVariety()


//--        HELPERS        --//


	/**
	* Prints info about the current uploaded file
	*
	* @param object	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function printExistingFileInfo(File $asset)
	{
		parent::printExistingFileInfo($asset);
		$existing = $asset->getExistingFile();
		if (!empty($existing)) {
			echo sprintf(translate('<b>Dimensions:</b> %1$s x %2$s pixels'), $existing['width'], $existing['height'])?><br/><?php

		}

	}//end printExistingFileInfo()


	/**
	* Get the info on the pre-uploaded file chosen by the user
	*
	* In this case the pre-uploaded file may have been uploaded by the image editor
	* applet, and we want to make sure the filename typed into the applet is used
	*
	* @param string	$prefix	prefix for form elements
	*
	* @return array
	* @access protected
	*/
	function getChosenFileInfo($prefix)
	{
		$info = parent::getChosenFileInfo($prefix);
		if (isset($_POST[$prefix.'_applet_filename']) && !empty($_POST[$prefix.'_applet_filename'])) {
			$info['name'] = $_POST[$prefix.'_applet_filename'];
			$info['tmp_name'] = SQ_TEMP_PATH.'/'.$_POST[$prefix.'_applet_temp_filename'];
		}
		return $info;

	}//end getChosenFileInfo()


	/**
	* Get a list of pre-uploaded files available for use
	*
	* In this case we restrict the options to be valid image file types
	* Returns value to be used with combo_box
	*
	* @param object	&$asset	The asset that might use the files
	*
	* @return array
	* @access protected
	*/
	function getPreUploadOptions(File $asset)
	{
		require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
		$options = Array();
		foreach (list_files(SQ_TEMP_PATH) as $filename) {
			if (in_array(get_file_type($filename), $asset->allowed_extensions)) {
				$options[$filename] = $filename;
			}
		}
		ksort($options);

		return $options;

	}//end getPreUploadOptions()



	/**
	* Regenerates the variety images after an upload or fix
	* It is the responsability of the caller to save the images attributes afterward
	*
	* @param object	&$asset	the file asset
	*
	* @return void
	* @access private
	*/
	function _recreateVarietyImages(&$asset)
	{
		$varieties = $asset->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['data'] as $varid => $data) {
				$variety = $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$asset->saveVariety($variety, TRUE, FALSE);
			}// end foreach
		}// end if

	}//end _recreateVarietyImages()


	/**
	* Extracts image metadata from an image file
	*
	* @param string	$filename	filename to extract metadata from
	* @param object $asset		the owner asset
	*
	* @return array|boolean
	* @access private
	*/
	function _extractImageMetadata($filename, $asset=NULL)
	{

		if (!SQ_TOOL_JPEG_TOOLKIT_ENABLED || !is_file($filename)) {
			return FALSE;
		}

		if (!is_file(SQ_TOOL_JPEG_TOOLKIT_PATH)) {
			trigger_error(translate('The path to image extraction tool ExifTool is invalid'), E_USER_WARNING);
			return FALSE;
		}

		$assetid = !is_null($asset) ? $asset->id : '';

		$meta = Array();

	    // Run a ExifTool tool to extract EXIF, XMP, IPTC, etc data from the image file
		// This will return the output in the JSON format
		$cmd = SQ_TOOL_JPEG_TOOLKIT_PATH.' -json '.$filename;
		$cmd = escapeshellcmd($cmd);
		// Run the command
		$log = Array('pipe', 'w');
	    $spec = Array(
				    0 => Array('pipe', 'r'), // stdin is a pipe that the child will read from
					1 => Array('pipe', 'w'), // stdout is a pipe that the child will write to
					2 => Array('pipe', 'w'), // stderr is a file to write to
				);

		$cwd = NULL;
		$env = Array();
		$process = proc_open($cmd, $spec, $pipes, $cwd, $env);

		if (is_resource($process)) {
			// If STDERR is empty, trying to get content from it will hang the process,
			// as it will wait forever for the data to be written on it, so set it as non-blocking
			stream_set_blocking($pipes[2], 0);
			fclose($pipes[0]);

			// Find the errors ... do NOT throw to output
			$errors = stream_get_contents($pipes[2]);
			fclose($pipes[2]);
			if (!empty($errors)) {
				$errors = array_unique(explode(PHP_EOL, $errors));
				log_error(sprintf(translate('Error occurred while getting image metadata for Image File asset #%1$s: %2$s'), $assetid, trim(implode(', ', $errors), ', ')), E_USER_WARNING);
			}//end if

			// Get the content from stdout after getting the content from stderr
			$output = stream_get_contents($pipes[1]);
			$array_output = json_decode_array(utf8_encode($output));

			fclose($pipes[1]);

			// If the command returned non-zero status
			if (proc_close($process) != 0 || is_null($array_output) || !isset($array_output[0])) {
				trigger_localised_error('CORE0324', sprintf(translate('Error occurred while getting image metadata for Image File asset #%1$s'), $assetid), E_USER_WARNING);
				return FALSE;
			}//end else

			// Log any error/warning
			if (isset($array_output[0]['Warning'])) {
				log_error(sprintf(translate('Error occurred while getting image metadata for Image File asset #%1$s: %2$s'), $assetid, $array_output[0]['Warning']), E_USER_WARNING);
			}

			// Get rid of irrelevant/sensitive data
			$irrelevant_data = Array('ExifToolVersion', 'SourceFile', 'Directory', 'FilePermissions');

			foreach($array_output[0] as $key => $val) {
				if (in_array($key, $irrelevant_data)) {
					continue;
				}

				if (is_array($val)) {
					array_walk($val, function(&$array_val, $array_key) {
						$array_val = str_replace("\0", '', $array_val);
					});
				} else {
					$val = str_replace("\0", '', $val);
				}

				$meta[strtolower($key)] = Array(
											'name' => $key,
											'value' => $val,
										);
			}//end foreach

		}//end if

		return $meta;

	}//end _extractImageMetadata()


}//end class

?>
