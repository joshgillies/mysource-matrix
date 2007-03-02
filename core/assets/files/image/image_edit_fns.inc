<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: image_edit_fns.inc,v 1.46.2.1 2007/03/02 03:47:12 hnguyen Exp $
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
* @version $Revision: 1.46.2.1 $
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
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		parent::paintFileUpload($asset, $o, $prefix);
		if ($asset->writeAccess('attributes')) {
			?><br/><span style="font-size: 10px;">You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><?php
		}
		if ((!$asset->id) && file_exists(SQ_FUDGE_PATH.'/image_editor/imagej.jar')) {
			// we are in the create screen, so offer the editor too
			echo '<p>';
			normal_button($prefix.'_use_editor_button', translate('core_use_image_editor_button'), 'this.parentNode.parentNode.parentNode.style.display=\'none\'; document.getElementById(\''.$prefix.'_hide_editor_button\').parentNode.parentNode.style.display=\'\';');
			echo '</p>';
		}

		return TRUE;

	}//end paintFileUpload()


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
	* @param object	&$asset	the file asset
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for form elements
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!parent::processFileUpload($asset, $o, $prefix, $info)) {
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

		if (!$asset->saveAttributes()) {
			trigger_localised_error('CORE0052', E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end processFileUpload()


	/**
	* Prints the image editor applet as it should appear in the create screen
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateEditor(&$asset, &$o, $prefix)
	{
		if (!$asset->id) {
			// we are in the create screen, so we'll print
			normal_button($prefix.'_hide_editor_button', translate('core_use_simple_upload_button'), 'this.parentNode.parentNode.style.display=\'none\'; document.getElementById(\''.$prefix.'_use_editor_button\').parentNode.parentNode.parentNode.style.display=\'\';', 'style="margin-bottom: 5px"');
			echo '<br />';
			$this->_paintApplet($asset, $o, $prefix);
		}
		return TRUE;

	}//end paintCreateEditor()


	/**
	* Process the results of the image editor in the create screen
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCreateEditor(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_applet_temp_filename']) && !empty($_POST[$prefix.'_applet_temp_filename'])) {
			$_POST[$prefix.'_server_file'] = $_POST[$prefix.'_applet_temp_filename'];
			// processFileChooser, called from File::create(), can now do the rest
		}
		return TRUE;

	}//end processCreateEditor()


	/**
	* Print the "edit image" screen
	* Returns TRUE if edit interface printed
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEditor(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$this->_paintApplet($asset, $o, $prefix);
			return TRUE;
		} else {
			?>
			<img src="<?php echo $GLOBALS['SQ_SYSTEM']->am->getRootURL().'?a='.$asset->id; ?>&t=<?php echo time(); ?>" />
			<?php
			return FALSE;
		}

	}//end paintEditor()


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
	function processEditor(&$asset, $o, $prefix)
	{
		if (isset($_POST[$prefix.'_applet_temp_filename'])) {
			$_POST[$prefix.'_server_file'] = $_POST[$prefix.'_applet_temp_filename'];
			// processServerFile can now do the rest
			$this->processFileChooser($asset, $o, $prefix);
		}
		return TRUE;

	}//end processEditor()


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
				$variety =& $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$edit_fns = $variety->getEditFns();

				$o->openSection($variety->name);
					$edit_fns->paintInlineInterface($variety, $o, $prefix.'_'.$variety->id);
					if ($wa) {
						$o->openField(translate('delete_question'));
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
				$variety =& $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$edit_fns = $variety->getEditFns();
				if ($edit_fns->processInlineInterface($variety, $o, $prefix.'_'.$variety->id)) {
					// update the public version with the new file
					if ($variety->checkFileState($asset->effectiveUnrestricted())) {
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
		$variety =& new Image_Variety($asset->id.':v0');

		$edit_fns = $variety->getEditFns();

		$o->openSection(translate('new_variety'));
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
		$variety =& new Image_Variety($asset->id.':v0');

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
	function printExistingFileInfo(&$asset)
	{
		parent::printExistingFileInfo($asset);
		$existing = $asset->getExistingFile();
		if (!empty($existing)) {
			echo translate('core_dimensions', $existing['width'], $existing['height'])?><br/><?php
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
	function getPreUploadOptions(&$asset)
	{
		require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
		$options = Array();
		foreach (list_files(SQ_TEMP_PATH) as $filename) {
			if (in_array(get_file_type($filename), $asset->allowed_extensions)) {
				$options[$filename] = $filename;
			}
		}
		return $options;

	}//end getPreUploadOptions()


	/**
	* Get a formatted string of extensions (eg 1,2,3 and 4)
	*
	* @param array	$extensions	array of extensions to format
	* @param string	$glue		word used to join last extension to the string (and, or etc)
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions, $glue='and')
	{
		if (empty($extensions) || !is_array($extensions)) {
			return '';
		}

		$count = count($extensions);
		$retval = '';

		for ($i = 0; $i < $count; $i++) {
			$ext = $extensions[$i];
			if ($i != 0 && ($i+1) == $count) {
				// the last in the list
				$retval .= " $glue ";
			} else if ($i != 0) {
				$retval .= ', ';
			}
			$retval .= $ext;
		}

		return $retval;

	}//end _getExtensionString()


	/**
	* Paint the Image Editor Applet, including the JS and hidden fields it needs
	*
	* We add our onsubmit function ourselves rather than letting the Backend_Outputter do it,
	* since the backend outputter will let us down in an asset builder
	* Return TRUE if the applet was printed OK
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function _paintApplet(&$asset, &$o, $prefix)
	{
		if (!file_exists(SQ_FUDGE_PATH.'/image_editor/imagej.jar')) {
			return FALSE;
		}
		$open_url = $asset->getURL();
		if (empty($open_url)) {
			$open_url = current_url(TRUE, TRUE).'?a='.$asset->id;
		}
		$open_url .= ((strpos($open_url, '?') === FALSE) ? '?' : '&').'t='.time();
		?>

		<script type="text/javascript"><!--
			function getAppletResult()
			{
				var applet = document.getElementById('<?php echo $prefix; ?>_image_editor');
				if (applet.parentNode.parentNode.style.display != 'none') {
					var tempFilename = applet.doUpload();
					if ((tempFilename !== null) && (tempFilename != '')) {
						document.getElementById('<?php echo $prefix; ?>_applet_temp_filename').value = tempFilename;
						document.getElementById('<?php echo $prefix; ?>_applet_filename').value = applet.getFilename();
						return true;
					} else {
						return false;
					}
				}
				return true;
			}

			forms = document.getElementsByTagName('FORM');
			if (forms[0].onsubmit) {
				var otherOnSubmit = forms[0].onsubmit;
				forms[0].onsubmit = function() { otherOnSubmit(); return getAppletResult(); }
			} else {
				forms[0].onsubmit = function() { return getAppletResult(); }
			}
			// -->
		</script>

		&nbsp;
		<input type="hidden" name="<?php echo $prefix; ?>_applet_temp_filename" id="<?php echo $prefix; ?>_applet_temp_filename" value="" />
		<input type="hidden" name="<?php echo $prefix; ?>_applet_filename" id="<?php echo $prefix; ?>_applet_filename" value="" />

		<applet id="<?php echo $prefix; ?>_image_editor" name="<?php echo $prefix; ?>_image_editor" archive="<?php echo sq_web_path('fudge'); ?>/image_editor/imagej.jar?t=<?php echo time(); ?>" code="ij.ImageJ.class" width="90%" height="350">
			<param name="SUBMIT_URL"
				   value="<?php echo sq_web_path('lib'); ?>/web/accept_file_upload.php?SESSION_ID=<?php echo session_id(); ?>&SESSION_KEY=<?php echo $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey(); ?>" />
			<param name="FILE_FIELD_NAME" value="<?php echo $prefix; ?>" />
		<?php
		if ($asset->id) {
			?>
			<param name="OPEN_URL" value="<?php echo $open_url; ?>" />
			<?php
		}
		?>
		</applet>
		<?php
		return TRUE;

	}//end _paintApplet()


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
				$variety =& $asset->getAsset($asset->id.':'.$varid);
				if (is_null($variety)) continue;
				$asset->saveVariety($variety, TRUE, FALSE);
			}// end foreach
		}// end if

	}//end _recreateVarietyImages()


}//end class

?>
