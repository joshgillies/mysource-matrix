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
* $Id: image_edit_fns.inc,v 1.27 2004/12/06 14:38:09 brobertson Exp $
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
* @version $Revision: 1.27 $
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
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
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
			normal_button($prefix.'_use_editor_button', 'Use Image Editor', 'this.parentNode.parentNode.parentNode.style.display=\'none\'; document.getElementById(\''.$prefix.'_hide_editor_button\').parentNode.parentNode.style.display=\'\';');
			echo '</p>';
		}
		return true;

	}//end paintFileUpload()


	/**
	* Prints the image editor applet as it should appear in the create screen
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateEditor(&$asset, &$o, $prefix)
	{
		if (!$asset->id) {
			// we are in the create screen, so we'll print
			normal_button($prefix.'_hide_editor_button', 'Use Simple Upload', 'this.parentNode.parentNode.style.display=\'none\'; document.getElementById(\''.$prefix.'_use_editor_button\').parentNode.parentNode.parentNode.style.display=\'\';', 'style="margin-bottom: 5px"');
			echo '<br />';
			$this->_paintApplet($asset, $o, $prefix);
		}
		return true;

	}//end paintCreateEditor()


	/**
	* Process the results of the image editor in the create screen
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @access public
	* @return boolean true
	*/
	function processCreateEditor(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_applet_temp_filename']) && !empty($_POST[$prefix.'_applet_temp_filename'])) {
			$_POST[$prefix.'_server_file'] = $_POST[$prefix.'_applet_temp_filename'];
			// processFileChooser, called from File::create(), can now do the rest
		}
		return true;

	}//end processCreateEditor()


	/**
	* Print the "edit image" screen
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @access public
	* @return boolean true if edit interface printed
	*/
	function paintEditor(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$this->_paintApplet($asset, $o, $prefix);
			return true;
		} else {
			?>
			<img src="<?php echo $GLOBALS['SQ_SYSTEM']->am->getRootURL().'?a='.$asset->id; ?>&t=<?php echo time(); ?>" />
			<?php
			return false;
		}

	}//end paintEditor()


	/**
	* Process the "edit image" screen
	*
	* @param object Image				&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @access public
	* @return boolean true if edit interface printed
	*/
	function processEditor(&$asset, $o, $prefix)
	{
		if (isset($_POST[$prefix.'_applet_temp_filename'])) {
			$_POST[$prefix.'_server_file'] = $_POST[$prefix.'_applet_temp_filename'];
			// processServerFile can now do the rest
			$this->processFileChooser($asset, $o, $prefix);
		}
		return true;

	}//end processEditor()


//--        HELPERS        --//


	/**
	* Prints info about the current uploaded file
	*
	* @param object File	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function printExistingFileInfo(&$asset)
	{
		parent::printExistingFileInfo($asset);
		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?><b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?> pixels<br/><?php
		}

	}//end printExistingFileInfo()


	/**
	* Get the info on the pre-uploaded file chosen by the user
	*
	* In this case the pre-uploaded file may have been uploaded by the image editor
	* applet, and we want to make sure the filename typed into the applet is used
	*
	* @param string						$prefix	prefix for form elements
	*
	* @access protected
	* @return array
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
	*
	* @param	&$asset		The asset that might use the files
	*
	* @access protected
	* @return array		value => value to be used with combo_box
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
	* @param array(string)	$extensions	array of extensions to format
	* @param string			$glue		word used to join last extension to the string (and, or etc)
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions, $glue='and')
	{
		if (empty($extensions) || !is_array($extensions)) return '';

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
	*
	* @param object Image				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @access private
	* @return boolean	true if the applet was printed OK
	*/
	function _paintApplet(&$asset, &$o, $prefix)
	{
		if (!file_exists(SQ_FUDGE_PATH.'/image_editor/imagej.jar')) return false;
		$open_url = $asset->getURL();
		if (empty($open_url)) $open_url = current_url(true, true).'?a='.$asset->id;
		$open_url .= ((strpos($open_url, '?') === false) ? '?' : '&').'t='.time();
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
		return true;

	}//end _paintApplet()


}//end class

?>
