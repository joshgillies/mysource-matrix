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
* $Id: image_edit_fns.inc,v 1.20 2004/11/10 02:15:08 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Edit_Fns extends File_Edit_Fns
{

	/**
	* Prints the interface for uploading a file
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
		if ($asset->writeAccess('attributes')) {
			file_upload($prefix);
			?><br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><br /><?php
		}

		$this->printExistingFileInfo($asset);
		return true;
	
	}//end paintFileUpload()



	/**
	* Processes the interface for uploading a file
	*
	* @param object Image				&$asset	the file asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!$asset->writeAccess('attributes')) {
			return false;
		}
		
		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}
		// check that the extension is allowed
		if (!$asset->validFile($info['name'])) return false;
		return parent::processFileUpload($asset, $o, $prefix, $info);

	}//end processFileUpload()



	/**
	* Prints the interface for uploading a file
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
		if (isset($_POST['imagej_submitted'])) {
			?>

SQ_GO_TO: <?php echo current_url().$GLOBALS['SQ_SYSTEM']->am->getRootURL(); ?>?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&asset_ei_screen=<?php echo $_REQUEST['asset_ei_screen'] ?>&assetid=<?php echo $asset->id; ?>

			<?php
		} elseif (!$asset->id) {
				?>
				<script language="JavaScript">
				<!--
				function setAllSectionsDisplay(display) {
					for (var i=0; i<20; i++) {
						if (i == <?php echo $o->getCurrentSectionNumber(); ?>) continue;
						var section = document.getElementById('backend_section_'+i);
						if (section != null) {
							section.style.display = display;
						}
					}
				}
				//-->
				</script>
				<input 
					type="button" 
					id = "ImageJButton"
					value="Use Image Editor"
					onclick="this.disabled = true;
							if (this.value == 'Use Image Editor') {
								this.value = 'Loading Applet...';
								setAllSectionsDisplay('none');
								document.getElementById('<?php echo $prefix; ?>_image_editor').style.display = 'block';
								setTimeout('var elt = document.getElementById(\'ImageJButton\'); elt.value = \'Use Simple Upload\'; elt.disabled=false;', 400);
							 } else {
							    this.value = 'Closing Applet...';
								setAllSectionsDisplay('');
								document.getElementById('<?php echo $prefix; ?>_image_editor').style.display = 'none';
								this.value = 'Use Image Editor';
								this.disabled=false;
							 }" />				
			<?php
				$this->_paintApplet($asset, $o, $prefix);
		}
		return true;
	
	}//end paintCreateEditor()

	function processCreateEditor(&$asset, &$o, $prefix)
	{
		if (isset($_POST['imagej_submitted'])) {
			$this->processFileUpload($asset, $o, $prefix);
		}
	}

	function paintEditor(&$asset, &$o, $prefix)
	{
		if (isset($_POST['imagej_submitted'])) {
			?>

SQ_GO_TO: <?php echo current_url().$GLOBALS['SQ_SYSTEM']->am->getRootURL(); ?>?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&asset_ei_screen=<?php echo $_REQUEST['asset_ei_screen'] ?>&assetid=<?php echo $asset->id; ?>

			<?php
		} elseif ($asset->writeAccess('attributes')) {
			$this->_paintApplet($asset, $o, $prefix);
		} else {
		?>
			<img src="<?php echo $GLOBALS['SQ_SYSTEM']->am->getRootURL().'?a='.$asset->id; ?>&t=<?php echo time(); ?>" />
		<?php
		}
		return false;
	}

	function processEditor(&$asset, $o, $prefix)
	{
		if (isset($_POST['imagej_submitted'])) {
			$this->processFileUpload($asset, $o, $prefix);
		}
	}


//--        HELPERS        --//

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


	
	function _paintApplet(&$asset, &$o, $prefix) 
	{
		$open_url = $asset->getURL();
		if (empty($open_url)) $open_url = current_url(true, true).'?a='.$asset->id;
		$open_url .= ((strpos($open_url, '?') === false) ? '?' : '&').'t='.time();
		?>
				<applet id="<?php echo $prefix; ?>_image_editor" name="<?php echo $prefix; ?>_image_editor" archive="<?php echo sq_web_path('fudge'); ?>/image_editor/imagej.jar?t=<?php echo time(); ?>" code="ij.ImageJ.class" width="90%" height="350" style="margin-top: 10px; <?php if (!$asset->id) echo 'display: none'; ?>">
					<param name="SUBMIT_URL" 
						   value="http://<?php echo $_SERVER['HTTP_HOST'] . $o->getCurrentLocation(); ?>&SESSION_ID=<?php echo session_id(); ?>&SESSION_KEY=<?php echo $GLOBALS['SQ_SYSTEM']->getUniqueSessionKey(); ?>" />
					<param name="FILE_FIELD_NAME" value="<?php echo $prefix; ?>" />
					<param name="REDIRECT_FRAME" value="sq_main" />
					<param name="FIELD_NAME_0" value="process_form" />
					<param name="FIELD_VALUE_0" value="1" />
					<param name="FIELD_NAME_1" value="am_form_submitted" />
					<param name="FIELD_VALUE_1" value="1" />
					<param name="FIELD_NAME_2" value="asset_action" />
					<param name="FIELD_VALUE_2" value="<?php echo ($asset->id) ? 'edit' : 'create'; ?>" />
					<param name="FIELD_NAME_3" value="imagej_submitted" />
					<param name="FIELD_VALUE_3" value="1" />
					<?php 
						if ($asset->id) {
					?>
					<param name="FIELD_NAME_4" value="backend_assetid" />
					<param name="FIELD_VALUE_4" value="<?php echo $asset->id; ?>" />
					<param name="OPEN_URL" value="<?php echo $open_url; ?>" />
					<?php
						 }
					?>
				</applet>
		<?php
	}


}//end class

?>
