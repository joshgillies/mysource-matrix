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
* $Id: content_type_image_prefs.inc,v 1.9 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Structure_Tree_Prefs
*
* Purpose
*
*    Looks after the creation the Image Content Type Preferences
*
* @author  Darren McKee <dmckee@squiz.net
* @version $Revision: 1.9 $
* @package MySource_Matrix
*/
class Content_Type_Image_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_CONTENT_TYPE_IMAGE_DEFAULTS'	=> Array(
																'name'			=> 'image_defaults_title',
																'description'	=> 'This preference allows you to change the default values',
																'default'		=> Array(
																					'caption'				=> 'Caption',
																					'caption_position'		=> 'bottom',
																					'use_caption_attr'		=> FALSE,
																					'width'					=> '400',
																					'height'				=> '400',
																					'fit_dimensions'		=> TRUE,
																					'constrain_proportions'	=> TRUE,
																					'on_click'				=> 'none',
																					'html_layout'			=> 'table',
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_FIELDS'		=> Array(
																'name'			=> 'image_fields_title',
																'description'	=> 'This preference allows you to specify which fields are displayed',
																'default'		=> Array(
																					'caption_options'	=> TRUE,
																					'caption_position'	=> TRUE,
																					'dimension_options'	=> TRUE,
																					'on_click'			=> TRUE,
																					'html_layout'		=> TRUE,
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_CLASSES'		=> Array(
																'name'			=> 'image_css_classes',
																'description'	=> 'This preference allows you to specify how the image is displayed',
																'default'		=> Array(
																					'surrounding_container'	=> 'image_table',
																					'image_table_cell'		=> 'image_table_cell',
																					'caption_table_cell'	=> 'caption_table_cell',
																					'image_tag'				=> 'inline_image',
																					'image_div'				=> 'image_div',
																					'caption_div'			=> 'caption_div',
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_PREVIEW'		=> Array(
																'name'			=> 'image_preview_css_classes',
																'description'	=> 'This preference allows you to specify how the image is displayed',
																'default'		=> Array(
																					'surrounding_container'	=> 'preview_container',
																					'preview_image_table'	=> 'preview_image_table',
																					'image_table_cell'		=> 'image_table_cell',
																					'image_tag'				=> 'preview_image',
																					'close_preview_cell'	=> 'close_cell',
																					'image_div'				=> 'preview_image_div',
																					'close_preview_div'		=> 'close_preview_div',
																				   ),
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_Image_Prefs($pref_file='')
	{

			$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		// Images for readonly mode on checkboxes
		$tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
		$cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="Yes" />';
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_DEFAULTS') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['name']));
			}
				$value_defaults = $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('caption'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[caption]', array_get_index($value_defaults, 'caption', ''), '30');
								} else {
									echo $value_defaults['caption'];
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('caption_position'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[caption_position]', Array('bottom' => 'Bottom', 'top' => 'Top'), FALSE, array_get_index($value_defaults, 'caption_position', ''));
									} else {
										echo ucwords($value_defaults['caption_position']);
									}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('use_caption_attribute'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									check_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[use_caption_attr]', '1', array_get_index($value_defaults, 'use_caption_attr', FALSE));
								} else {
									echo ($value_defaults['use_caption_attr'] == TRUE) ? $tick : $cross;
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('width'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[width]', array_get_index($value_defaults, 'width', ''), '10');
								} else {
									echo $value_defaults['width'];
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('height'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[height]', array_get_index($value_defaults, 'height', ''), '10');
								} else {
									echo $value_defaults['height'];
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('constrain_proportions'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									check_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[constrain_proportions]', '1', array_get_index($value_defaults, 'constrain_proportions', FALSE));
								} else {
									echo (array_get_index($value_defaults, 'constrain_proportions', FALSE) == TRUE) ? $tick : $cross;
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('fit_dimensions'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									check_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[fit_dimensions]', '1', array_get_index($value_defaults, 'fit_dimensions', FALSE));
								} else {
									echo (array_get_index($value_defaults, 'fit_dimensions', FALSE) == TRUE) ? $tick : $cross;
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('on_click'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									$GLOBALS['SQ_SYSTEM']->am->includeAsset('content_type_image');
									$dummy_asset = new Content_Type_Image();
									$onclick = $dummy_asset->getAttribute('on_click');
									$onclick->value = $value_defaults['on_click'];
									$onclick->paint('content_type_image_on_click');
								} else {
									echo ucwords($value_defaults['on_click']);
								}
							?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('html_layout'); ?>:</td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[html_layout]', Array('table' => 'Table', 'div' => 'Div'), FALSE, array_get_index($value_defaults, 'html_layout', ''));
								} else {
									echo ucwords($value_defaults['html_layout']);
								}
							?>
						</td>
					</tr>
				</table>
				<?php
			if (is_null($pref)) $o->closeField();
		}//end if
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_FIELDS') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['name']));
			}
			$fields = $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default'];
			?>
			<table class="sq-backend-table" style="width: 200px;">
				<?php
				foreach ($fields as $field_name => $value) {
					?>
					<tr>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									check_box('SQ_CONTENT_TYPE_IMAGE_FIELDS['.$field_name.']', '1', $value, '', 'id="image_content_'.$field_name.'"');
									label(ucwords(str_replace('_', ' ', $field_name)), 'image_content_'.$field_name);
								} else {
									echo ($value == TRUE) ? $tick : $cross;
									?>
									&nbsp;&nbsp;&nbsp;
									<?php
									echo ucwords(str_replace('_', ' ', $field_name));
								}
							?>
						</td>
					</tr>
				<?php
			}
			?>
			</table>
			<?php
		}//end if
		if (is_null($pref)) $o->closeField();
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_CLASSES') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_CLASSES']['name']));
			}
			$classes = $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_CLASSES']['default'];
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<?php
				foreach ($classes as $name => $class) {
					?>
					<tr>
						<td class="sq-backend-table-header"><?php echo ucwords(str_replace('_', ' ', $name)); ?></td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_CONTENT_TYPE_IMAGE_CLASSES['.$name.']', $class);
								} else {
									echo $class;
								}
							?>
						</td>
					</tr>
				<?php
				}
				?>
			</table>
			<?php
		}//end if
		if (is_null($pref)) $o->closeField();

		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_PREVIEW') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['name']));
			}
			$classes = $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['default'];
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<?php
				foreach ($classes as $name => $class) {
					?>
					<tr>
						<td class="sq-backend-table-header"><?php echo ucwords(str_replace('_', ' ', $name)); ?></td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_CONTENT_TYPE_IMAGE_PREVIEW['.$name.']', $class);
								} else {
									echo $class;
								}
							?>
						</td>
					</tr>
				<?php
				}
				?>
			</table>
			<?php
		}//end if
		if (is_null($pref)) $o->closeField();


		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_DEFAULTS') {
			$posted_values = $_POST['SQ_CONTENT_TYPE_IMAGE_DEFAULTS'];
			if (!isset($posted_values['use_caption_attr'])) {
				$posted_values['use_caption_attr'] = FALSE;
			}
			if (!isset($posted_values['fit_dimensions'])) {
				$posted_values['fit_dimensions'] = FALSE;
			}
			if (!isset($posted_values['constrain_proportions'])) {
				$posted_values['constrain_proportions'] = FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('content_type_image');
			$dummy_asset = new Content_Type_Image();
			$onclick = $dummy_asset->getAttribute('on_click');
			$onclick->process('content_type_image_on_click');
			$posted_values['on_click'] = $onclick->value;
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['default'] = $posted_values;
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_FIELDS') {
			foreach ($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default'] as $name => $val) {
				$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default'][$name] = isset($_POST['SQ_CONTENT_TYPE_IMAGE_FIELDS'][$name]);
			}
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_CLASSES') {
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_CLASSES']['default'] = $_POST['SQ_CONTENT_TYPE_IMAGE_CLASSES'];
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_PREVIEW') {
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['default'] = $_POST['SQ_CONTENT_TYPE_IMAGE_PREVIEW'];
		}
		return TRUE;

	}//end processBackend()


}//end class

?>
