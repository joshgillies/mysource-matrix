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
* $Id: image_variety_edit_fns.inc,v 1.7 2005/03/03 16:39:19 brobertson Exp $
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
* @version $Revision: 1.7 $
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
									'details' => Array(
													'name'			=> 'Details',
													'force_unlock'	=> false,
													'lock_type'		=> 'none',
												),
								);

	}//end constructor


	/**
	* Paint the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInlineInterface(&$asset, &$o, $prefix)
	{
		require_once 'XML/Tree/Node.php';
		$wa = $asset->writeAccess('attributes');

		if (!$asset->id && !$wa) return false;

		$variety_type = $asset->attr('variety_type');
		$constrain    = $asset->attr('constrain');

		$int_edit_params  = new XML_Tree_Node('int', '', Array('width' => 8));
		$bool_edit_params = new XML_Tree_Node('boolean', '', Array('true_text' => 'Yes', 'false_text' => 'No'));
		$sel_edit_params  = new XML_Tree_Node('selection', '', Array('style' => 'box', 'columns' => '3', 'extras' => ''));

		if ($asset->name == '') {
			$o->openField('Name');
				$text_edit_params = new XML_Tree_Node('text', '', Array('width' => 15));
				$attr = &$asset->getAttribute('name');
				$attr->setEditParams($text_edit_params);
				$attr->paint($prefix.'_'.$attr->name, !$wa);
				unset($attr);
			$o->closeField();
		}

		$o->openField('Variety Type');
			$js_prefix = preg_replace('/[^a-z0-9_]/', '_', $prefix);
		?>
			<script type="text/javascript">
				<!-- //

				function <?php echo $js_prefix; ?>_update_visible_fields() {

					var variety_type = get_form_element_value("<?php echo $prefix; ?>_variety_type");
					var constrain    = get_form_element_value("<?php echo $prefix; ?>_constrain");

					document.getElementById('<?php echo $prefix; ?>_field_constrain').style.display	= (variety_type == 'resize') ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_width').style.display		= (variety_type == 'resize' && constrain != 'height') ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_height').style.display	= (variety_type == 'resize' && constrain != 'width')  ? '' : 'none';
					document.getElementById('<?php echo $prefix; ?>_field_upload').style.display	= (variety_type == 'upload') ? '' : 'none';

				}// end <?php echo $prefix; ?>_update_visible_fields()

				// -->
			</script>
		<?php
			$sel_edit_params->attributes['extras'] = 'onclick="javascript: '.$js_prefix.'_update_visible_fields();"';
			$attr = &$asset->getAttribute('variety_type');
			$attr->setEditParams($sel_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();
		
		$o->openField('Constrain by', '', '', ($variety_type != 'resize'), $prefix.'_field_constrain');
			$sel_edit_params->attributes['extras'] = 'onclick="javascript: '.$js_prefix.'_update_visible_fields();"';
			$attr = &$asset->getAttribute('constrain');
			$attr->setEditParams($sel_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();

		$o->openField('Width', '', '', ($variety_type != 'resize' || $constrain == 'height'), $prefix.'_field_width');
			$attr = &$asset->getAttribute('width');
			$attr->setEditParams($int_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();
		
		$o->openField('Height', '', '', ($variety_type != 'resize' || $constrain == 'width'), $prefix.'_field_height');
			$attr = &$asset->getAttribute('height');
			$attr->setEditParams($int_edit_params);
			$attr->paint($prefix.'_'.$attr->name, !$wa);
			unset($attr);
		$o->closeField();
		
		$o->openField('Upload', '', '', ($variety_type != 'upload'), $prefix.'_field_upload');
			if ($wa) {
				file_upload($prefix.'_upload_file');
			}
		$o->closeField();

		if ($asset->id) {
			$o->openField('Current Variety');
				?>
					<a href="<?php echo current_url(true, true).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank">Click to open in a new window</a>
				<?php
			$o->closeField();
		}

		return true;

	}//end paintInlineValueInterface();


	/**
	* Process the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are processing
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInlineInterface(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$changes_made = false;

		$vars = Array('variety_type', 'constrain', 'width', 'height');
		if ($asset->attr('name') == '') array_unshift($vars, 'name');

		foreach($vars as $attr_name) {
			$attr = &$asset->getAttribute($attr_name);
			$attr->process($prefix.'_'.$attr->name);
			if ($attr->processed && $asset->setAttrValue($attr->name, $attr->value)) {
				$changes_made = true;
			}
		}

		if ($asset->_imageid && $asset->attr('variety_type') == 'upload') {
			if ($this->_processUploadedFile($asset, $prefix)) {
				$changes_made = true;
			}
		}

		if (!$changes_made) return false;
		else return $asset->saveAttributes();

	}//end processInlineValueInterface();


	/**
	* Takes care of processing any uploaded file
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are processing
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @access public
	* @return boolean
	*/
	function _processUploadedFile(&$asset, $prefix)
	{
		if (empty($asset->_imageid)) return false;
		if ($asset->attr('name') == '') return false;

		$info = get_file_upload_info($prefix.'_upload_file');

		// return on failed or no upload
		if ($info === false || empty($info)) return false;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();
		if (!is_dir($asset->data_path) && !create_directory($asset->data_path)) return false;

		$image = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_imageid);
		if (is_null($image)) return false;

		$img_ext = get_file_type($image->name);
		$upload_ext = get_file_type($info['name']);

		if ($img_ext != $upload_ext) {
			trigger_error('An image uploaded for a variety MUST be of the same type as the original image.', E_USER_WARNING);
			return false;
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
					trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
					return false;
				}
			}

		}// endif

		if (!commit_file_upload($prefix.'_upload_file', $d_new_file, true, 0)) return false;

		$up_to_date = $fv->upToDate($d_new_file);

		// file not in repository
		if ($up_to_date & FUDGE_FV_NOT_CHECKED_OUT) {
			if (!$fv->add($asset->data_path_suffix, $d_new_file, '', false)) return false;

			if (!$fv->checkOut($asset->data_path_suffix.'/'.$d_new_filename, $asset->data_path)) {
				trigger_error('Update failed checking-out latest version of file', E_USER_WARNING);
				return false;
			}

		} elseif ($up_to_date & FUDGE_FV_MODIFIED) {
			if (!$fv->commit($d_new_file, '', false)) return false;

		}

		// if the filename has changed then we need to update the look ups
		if (!$asset->setAttrValue('filename', $d_new_filename)) return false;

		return true;

	}// end _processUploadedFile()


}//end class

?>
