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
* $Id: image_variety_edit_fns.inc,v 1.23 2013/07/31 04:23:46 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Image_Variety_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Variety_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Image_Variety_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(
									'details'	=> Array(
													'name'			=> translate('Details'),

													'force_unlock'	=> FALSE,
													'lock_type'		=> 'none',
												   ),
								);

	}//end constructor


	/**
	* Paint the interface for the image variety
	*
	* @param Image_Variety		$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintInlineInterface(Image_Variety $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if (!$asset->id && !$wa) {
			return FALSE;
		}

		$variety_type = $asset->attr('variety_type');
		$constrain    = $asset->attr('constrain');

		$int_edit_params  = new SimpleXMLElement('<int       width="8" />');
		$bool_edit_params = new SimpleXMLElement('<boolean   true_text="Yes" false_text="no" />');
		$sel_edit_params  = new SimpleXMLElement('<selection style="box" columns="5" extras="" />');

		if ($asset->name == '') {
			$o->openField(translate('Name'));

				$text_edit_params = new SimpleXMLElement('<text width="15" />');
				$attr = $asset->getAttribute('name');
				$attr->setEditParams($text_edit_params);
				$attr->paint($prefix.'_'.$attr->name, !$wa);
				unset($attr);
			$o->closeField();
		}

		$o->openField(translate('Variety Type'));

			$js_prefix = preg_replace('/[^a-z0-9_]/', '_', $prefix);
		?>
			<script type="text/javascript">
				<!-- //

				function <?php echo $js_prefix; ?>_update_visible_fields() {

					var variety_type = get_form_element_value("<?php echo $prefix; ?>_variety_type");
					var constrain    = get_form_element_value("<?php echo $prefix; ?>_constrain");

					document.getElementById('<?php echo $prefix; ?>_field_constrain').style.display	= (variety_type == 'resize') ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_width').style.display		= (variety_type == 'resize' && (constrain == 'width' || constrain == 'none')) ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_height').style.display	= (variety_type == 'resize' && (constrain == 'height' || constrain == 'none'))  ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_dimension').style.display	= (variety_type == 'resize' && (constrain == 'dimension' || constrain == 'centre_weighted_square'))  ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_upload').style.display	= (variety_type == 'upload') ? '' : 'none';
					return true;

				}// end <?php echo $prefix; ?>_update_visible_fields()

				// -->
			</script>
		<?php
			$sel_edit_params->attributes()->extras = 'onclick="return '.$js_prefix.'_update_visible_fields();"';
			$attr = $asset->getAttribute('variety_type');
			$attr->setEditParams($sel_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField(translate('Constrain by'), '', '', ($variety_type != 'resize'), $prefix.'_field_constrain');

			$sel_edit_params->attributes()->extras = 'onclick="return '.$js_prefix.'_update_visible_fields();"';
			$attr = $asset->getAttribute('constrain');
			$attr->setEditParams($sel_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField(translate('Width'), '', '', ($variety_type != 'resize' || $constrain != 'Width'), $prefix.'_field_width');

			$attr = $asset->getAttribute('width');
			$attr->setEditParams($int_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField(translate('Height'), '', '', ($variety_type != 'resize' || $constrain != 'Height'), $prefix.'_field_height');

			$attr = $asset->getAttribute('height');
			$attr->setEditParams($int_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField(translate('Dimension'), '', translate('The maximum size of the largest dimension'), ($variety_type != 'resize' || ($constrain != 'Dimension' && $constrain != 'centre_weighted_square')), $prefix.'_field_dimension');


			$attr = $asset->getAttribute('dimension');
			$attr->setEditParams($int_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField(translate('Upload'), '', '', ($variety_type != 'Upload'), $prefix.'_field_upload');

			if ($wa) file_upload($prefix.'_upload_file');
		$o->closeField();

		if ($asset->id) {
			$o->openField(translate('Current Variety'));

				?>
					<a href="<?php echo current_url(TRUE, TRUE).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank"><?php echo translate('Click to open in a new window'); ?></a>

				<?php
			$o->closeField();
		}

		return TRUE;

	}//end paintInlineInterface()


	/**
	* Process the interface for the image variety
	*
	* @param Image_Variety		$asset	the asset whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processInlineInterface(Image_Variety $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$changes_made = FALSE;

		$vars = Array('variety_type', 'constrain', 'width', 'height', 'dimension');
		if ($asset->attr('name') == '') {
			array_unshift($vars, 'name');
		}

		foreach ($vars as $attr_name) {
			$attr = $asset->getAttribute($attr_name);
			$attr->process($prefix.'_'.$attr->name);
			if ($attr->processed && $asset->setAttrValue($attr->name, $attr->value)) {
				$changes_made = TRUE;
			}
		}

		if ($asset->_imageid && $asset->attr('variety_type') == 'upload') {
			$info = get_file_upload_info($prefix.'_upload_file');
			if ($this->processUploadedFile($asset, $info)) {
				$changes_made = TRUE;
			}
		}

		if (!$changes_made) {
			return FALSE;
		} else {
			return $asset->saveAttributes();
		}

	}//end processInlineInterface()


	/**
	* Takes care of processing any uploaded file
	*
	* If you want to process an upload of a file that was not uploaded through PHP
	* (possible because of FTP or it was in a tar archive), ensure you pass the information
	* about the file in the $info array.
	* The fields required are:
	*     name              => the filename without path (eg file.txt)
	*     tmp_name          => the location of the file on the file system
	*     non_uploaded_file => must equal TRUE
	*
	* @param Image_Variety	$asset	the asset whose interface we are processing
	* @param array			$info	info about the uploaded file
	*
	* @return boolean
	*/
	public function processUploadedFile(Image_Variety $asset, Array $info=Array())
	{
		if (empty($asset->_imageid)) return FALSE;
		if ($asset->attr('name') == '') return FALSE;

		// return on failed or no upload
		if ($info === FALSE || empty($info)) {
			return FALSE;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
		if (!is_dir($asset->data_path) && !create_directory($asset->data_path)) {
			return FALSE;
		}

		$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_imageid);
		if (is_null($image)) return FALSE;

		$img_ext = get_file_type($image->name);
		$upload_ext = get_file_type($info['name']);

		if ($img_ext != $upload_ext) {
			trigger_localised_error('CORE0003', translate('An image uploaded for a variety MUST be of the same type as the original image.'), E_USER_WARNING);
			return FALSE;
		}

		$d_old_filename = $asset->attr('filename');
		$d_new_filename = $asset->attr('name').'.'.$img_ext;
		$d_old_file     = $asset->data_path.'/'.$d_old_filename;
		$d_new_file     = $asset->data_path.'/'.$d_new_filename;

		// if the filename has changed then we need to remove the old file
		if ($d_old_filename != $d_new_filename) {
			$asset->deleteImage();

		} else {
			// make sure we have the latest version checked out if we are modifying the file
			$up_to_date = $fv->upToDate($d_new_file);
			if ($up_to_date & FUDGE_FV_OLD_VERSION) {
				if (!$fv->checkOut($asset->data_path_suffix.'/'.$d_new_filename, $asset->data_path)) {
					trigger_localised_error('CORE0103', translate('Update failed checking-out latest version of file'), E_USER_WARNING);
					return FALSE;
				}
			}

		}// endif

		// copy over the new uploaded file
		if (is_uploaded_file($info['tmp_name'])) {
			// a normal uploaded file
			if (!move_uploaded_file($info['tmp_name'], $d_new_file)) {
				trigger_localised_error('CORE0038', E_USER_WARNING ,$info['tmp_name'], $d_new_file);
				return FALSE;
			}
		} else if (isset($info['non_uploaded_file']) && $info['non_uploaded_file'] === TRUE) {
			// we have special permission to move a non-uploaded file
			if (!copy($info['tmp_name'], $d_new_file)) {
				trigger_localised_error('CORE0036', sprintf(translate('Failed copying non-uploaded file (%1$s) to destination directory (%2$s)'), $info['tmp_name'], $d_new_file), E_USER_WARNING);
				return FALSE;
			}
		} else {
			trigger_localised_error('CORE0067', translate('No file uploaded or non-uploaded, unable to process file upload'), E_USER_ERROR);
			return FALSE;
		}

		$up_to_date = $fv->upToDate($d_new_file);

		// file not in repository
		if ($up_to_date & FUDGE_FV_NOT_CHECKED_OUT) {
			if (!$fv->add($asset->data_path_suffix, $d_new_file, '')) {
				return FALSE;
			}

			if (!$fv->checkOut($asset->data_path_suffix.'/'.$d_new_filename, $asset->data_path)) {
				trigger_localised_error('CORE0103', translate('Update failed checking-out latest version of file'), E_USER_WARNING);
				return FALSE;
			}

		} else if ($up_to_date & FUDGE_FV_MODIFIED) {
			if (!$fv->commit($d_new_file, '')) return FALSE;

		}

		// if the filename has changed then we need to update the look ups
		if (!$asset->setAttrValue('filename', $d_new_filename)) {
			return FALSE;
		}

		return TRUE;

	}//end processUploadedFile()


}//end class

?>
