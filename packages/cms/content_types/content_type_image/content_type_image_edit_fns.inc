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
* $Id: content_type_image_edit_fns.inc,v 1.13.6.1 2013/09/18 05:21:23 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Image_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.13.6.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Image_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the backend interface for this Image
	*
	* @param object	&$asset	the raw html content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$current_link = $am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'image_link');

			if (empty($current_link)) {
				$current_link['minorid'] = '';
			}

			$display_fields = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CONTENT_TYPE_IMAGE_FIELDS');
			$defaults = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CONTENT_TYPE_IMAGE_DEFAULTS');
			$current_settings = $asset->attr('display_settings');

			if (empty($current_settings)) {
				$asset->setAttrValue('on_click', $defaults['on_click']);
				$asset->saveAttributes();
			}
			unset($defaults['on_click']);
			// Sets default values from the user prefs
			foreach ($defaults as $id => $value) {
				// This may need to be changed, so that if the defaults are changed, the values of each
				// Image follow suit. At the moment this will set the defaults on the first edit.
				if (!isset($current_settings[$id])) {
					$current_settings[$id] = $value;
				}
			}
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/JsHttpConnector.js' ?>"></script>
			<script type="text/javascript">
				<?php echo $prefix; ?>_alt_text = '';
				<?php echo $prefix; ?>_width_text = '';
				<?php echo $prefix; ?>_height_text = '';
				<?php echo $prefix; ?>_proportion_width = 1;
				<?php echo $prefix; ?>_proportion_height = 1;

				function <?php echo $prefix; ?>_populateImageInfo(responseText) {
					var imageInfo = var_unserialise(responseText);

					<?php echo $prefix; ?>_alt_text = imageInfo['alt'];
					<?php echo $prefix; ?>_width_text = imageInfo['width'];
					<?php echo $prefix; ?>_height_text = imageInfo['height'];
					<?php echo $prefix; ?>_proportion_width = imageInfo['height'] / imageInfo['width'];
					<?php echo $prefix; ?>_proportion_height = imageInfo['width'] / imageInfo['height'];
					return;
				};

				function <?php echo $prefix; ?>setImageInfo(id)
				{
					// put a random no in the url to overcome any caching
					var url = '<?php echo sq_web_path('root_url').'/'.SQ_CONF_BACKEND_SUFFIX; ?>/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid=' + escape(id) + '&asset_ei_screen=image_info&ignore_frames=1&t=' + Math.random() * 1000;
					JsHttpConnector.submitRequest(url, <?php echo $prefix; ?>_populateImageInfo);
				};

				function <?php echo $prefix; ?>_id_changed()
				{
					var preview = document.getElementById('<?php echo $prefix; ?>_preview');
					var new_id = document.getElementById('<?php echo $prefix; ?>_image_selection[assetid]').value;
					if (new_id == '' || new_id == '0') {
						preview.innerHTML = 'No Image Selected';
						return;
					}
					<?php echo $prefix; ?>setImageInfo(new_id);
					preview.innerHTML = '<img id="<?php echo $prefix; ?>_prev_image" src="./?a=' + new_id + '" />';
					<?php echo $prefix; ?>_fit_changed();

				}//end id_changed()

				function <?php echo $prefix; ?>_prop_changed(elt)
				{
					if (elt.checked == true) {
						<?php echo $prefix; ?>_dimension_changed(true);
					}

				}//end prop_changed()

				function <?php echo $prefix; ?>_dimension_changed(use_width)
				{
					var use_props = document.getElementById('<?php echo $prefix; ?>_proportions').checked;
					var height = 0;
					var width = 0;
					if (use_props == true) {
						if (use_width) {
							width = document.getElementById('<?php echo $prefix; ?>_width').value;
							if (width == '') {
								width = '400';
								document.getElementById('<?php echo $prefix; ?>_width').value = width;
							}
							height = Math.round(width * <?php echo $prefix; ?>_proportion_width);
							document.getElementById('<?php echo $prefix; ?>_height').value = height;
						} else {
							height = document.getElementById('<?php echo $prefix; ?>_height').value;
							if (height ==  '') {
								height = '400';
								document.getElementById('<?php echo $prefix; ?>_height').value = height;
							}
							width = Math.round(height * <?php echo $prefix; ?>_proportion_height);
							document.getElementById('<?php echo $prefix; ?>_width').value = width;
						}
					} else {
						width = document.getElementById('<?php echo $prefix; ?>_width').value;
						width = (width == '') ? '400' : width;
						height = document.getElementById('<?php echo $prefix; ?>_height').value;
						height = (height == '') ? '400' : height;
					}
					var td = document.getElementById('<?php echo $prefix; ?>_preview');
					td.style.width = width;
					td.style.height = height;
					<?php echo $prefix; ?>_fit_changed();
				}

				function <?php echo $prefix; ?>_fit_changed()
				{
					var checked = document.getElementById('<?php echo $prefix; ?>_fit').checked;
					image = document.getElementById('<?php echo $prefix; ?>_prev_image');
					if (image == null) return;
					if (checked == true) {
						var td = document.getElementById('<?php echo $prefix; ?>_preview');
						image.style.width = td.style.width;
						image.style.height = td.style.height;
					} else {
						image.style.width = '';
						image.style.height = '';
					}
				}

				function <?php echo $prefix; ?>_change_caption(mode)
				{
					var cap_box = document.getElementById('<?php echo $prefix; ?>_caption');
					if (mode == 'original') {
						cap_box.disabled = '';
					} else {
						cap_box.disabled = 'disabled';
					}
				}
			</script>
			<table>
				<tr>
				<td id="<?php echo $prefix; ?>_preview" style="width: <?php echo $current_settings['width']; ?>px; height: <?php echo $current_settings['height']; ?>px; border: 1px solid black; text-align: center; margin-left: auto; margin-right: auto;" colspan="2"></td>
				</tr>
			</table>
			<table>
			<tr>
				<td class="sq-backend-table-header" style="width: 250px;">Image:</td>
				<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($asset->writeAccess('links')) {
						asset_finder($prefix.'_image_selection', $current_link['minorid'], Array('image' => 'D', 'image_variety' => 'D'), 'sq_sidenav', FALSE, $prefix.'_id_changed');
					}
					?>
				</td>
			</tr>
			<tr <?php echo (!$display_fields['caption_options']) ? 'style="display:none;"' : ''; ?>>
				<td class="sq-backend-table-header" style="width: 250px;"><?php echo translate('caption'); ?>:</td>
				<td class="sq-backend-table-cell">
					<?php radio_button($prefix.'_caption_options', '0', !$current_settings['use_caption_attr'], $prefix.'_change_caption(\'original\');'); ?>
					<?php hidden_field($prefix.'_caption_hidden', $current_settings['caption']); ?>
					<?php $disabled_val = $current_settings['use_caption_attr'] ? ' disabled="disabled"' : ''; ?>
					<?php text_box($prefix.'_caption', $current_settings['caption'], '50', '0', FALSE, 'onchange="document.getElementById(\''.$prefix.'_caption_hidden\').value = document.getElementById(\''.$prefix.'_caption\').value;" '.$disabled_val); ?>
					<?php radio_button($prefix.'_caption_options', '1', $current_settings['use_caption_attr'], $prefix.'_change_caption(\'alt\');'); ?>
					<?php label('Use Image Caption Attribute', $prefix.'_caption[\'alt\']'); ?>
				</td>
			</tr>
			<tr <?php echo (!$display_fields['caption_position']) ? 'style="display:none;"' : ''; ?>>
				<td class="sq-backend-table-header">Caption Position:</td>
				<td class="sq-backend-table-cell">
					<?php radio_button($prefix.'_caption_position', 'top', $current_settings['caption_position'] == 'top', '', 'id="'.$prefix.'_cap_pos_top_id'); ?>
					<?php label('Top', $prefix.'_cap_pos_top_id'); ?>
					<?php radio_button($prefix.'_caption_position', 'bottom', $current_settings['caption_position'] == 'bottom', '', 'id="'.$prefix.'_cap_pos_bottom_id'); ?>
					<?php label('Bottom', $prefix.'_cap_pos_bottom_id'); ?>
				</td>
			</tr>
			<tr <?php echo (!$display_fields['dimension_options']) ? 'style="display:none;"' : ''; ?>>
				<td class="sq-backend-table-header" style="width: 100px;">Dimensions</td>
				<td class="sq-backend-table-cell">
					Width: <?php text_box($prefix.'_width', $current_settings['width'], '4', '0', FALSE, 'onchange="'.$prefix.'_dimension_changed(true);"'); ?>px
					&nbsp;&nbsp;Height: <?php text_box($prefix.'_height', $current_settings['height'], '4', '0', FALSE, 'onchange="'.$prefix.'_dimension_changed(false);"'); ?>px
					&nbsp;&nbsp;<?php check_box($prefix.'_proportions', '1', $current_settings['constrain_proportions'], $prefix.'_prop_changed(this);'); ?>
					<?php label('Constrain Proportions', $prefix.'_prop'); ?>
					&nbsp;&nbsp;<?php check_box($prefix.'_fit', '1', $current_settings['fit_dimensions'], $prefix.'_fit_changed();'); ?>
					<?php label('Shrink/Stretch to Dimensions', $prefix.'_fit'); ?>
				</td>
			</tr>
			<tr <?php echo (!$display_fields['on_click']) ? 'style="display:none;"' : ''; ?>>
				<td class="sq-backend-table-header">On Clicking:</td>
				<td class="sq-backend-table-cell">
				<?php
					$onclick = $asset->getAttribute('on_click');
					$onclick->paint($prefix);
				?>
				</td>
			</tr>
			<tr <?php echo (!$display_fields['html_layout']) ? 'style="display:none;"' : ''; ?>>
				<td class="sq-backend-table-header">HTML Layout:</td>
				<td class="sq-backend-table-cell">
					<?php radio_button($prefix.'_html_layout', 'table', $current_settings['html_layout'] == 'table', '', 'id="'.$prefix.'_html_layout_table_id'); ?>
					<?php label('Table', $prefix.'_html_layout_table_id'); ?>
					<?php radio_button($prefix.'_html_layout', 'div', $current_settings['html_layout'] == 'div', '', 'id="'.$prefix.'_html_layout_div_id'); ?>
					<?php label('Div', $prefix.'_html_layout_div_id'); ?>
				</td>
			</tr>
			
			<?php
			?>
			</table>
			<script type="text/javascript">
				<?php echo $prefix; ?>_id_changed();
			</script>
			<?php
		}//end if
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Image
	* Creates a container with the image, and all selected options applied
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the wysiwyg content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			$current_link = $am->getLink($asset->id, SQ_LINK_NOTICE, '', FALSE, 'image_link');

			// Process Image Link
			$img_assetid = NULL;
			if (isset($_POST[$prefix.'_image_selection']['assetid'])) {
				$img_assetid = $_POST[$prefix.'_image_selection']['assetid'];
				// imageid changed, delete existing
				if (!empty($current_link) && $img_assetid != $current_link['minorid']) {
					$am->deleteAssetLink($current_link['linkid']);
				}
				// new imageid, but no link or different link, create new link
				if (!empty($img_assetid) &&
					(!isset($current_link['minorid']) || $img_assetid != $current_link['minorid'])
				) {
					$new_screen_id = $_POST[$prefix.'_image_selection']['assetid'];
					if (!empty($new_screen_id) || ($new_screen_id != '0')) {
						$image = $am->getAsset($new_screen_id);
						$new_link_id = $asset->createLink($image, SQ_LINK_NOTICE, 'image_link');
						$current_link = $am->getLinkById($new_link_id);
					}
				}
			}

			// Process the attributes
			$current_settings = $asset->attr('display_settings');
			$current_settings['width'] = isset($_POST[$prefix.'_width']) ? $_POST[$prefix.'_width'] : '400';
			$current_settings['height'] = isset($_POST[$prefix.'_height']) ? $_POST[$prefix.'_height'] : '400';
			$current_settings['caption'] = isset($_POST[$prefix.'_caption_hidden']) ? $_POST[$prefix.'_caption_hidden'] : 'Caption';
			$current_settings['fit_dimensions'] = isset($_POST[$prefix.'_fit']);
			$current_settings['use_caption_attr'] = array_get_index($_POST, $prefix.'_caption_options', FALSE);
			if (isset($_POST[$prefix.'_caption_position'])) {
				$current_settings['caption_position'] = $_POST[$prefix.'_caption_position'];
			} else {
				$current_settings['caption_position'] = 'bottom';
			}
			if (isset($_POST[$prefix.'_html_layout'])) {
				$current_settings['html_layout'] = $_POST[$prefix.'_html_layout'];
			} else {
				$current_settings['html_layout'] = 'table';
			}
			$current_settings['constrain_proportions'] = isset($_POST[$prefix.'_proportions']);

			$onclick = $asset->getAttribute('on_click');
			$onclick->process($prefix);

			$on_click_option = $onclick->value;
			$image_linked = FALSE;
			if ($on_click_option == 'new_window' || $on_click_option == 'current_window') {
				$image_linked = TRUE;
			}

			$asset->setAttrValue('display_settings', $current_settings);
			$asset->setAttrValue('on_click', $onclick->value);

			if (empty($current_link) || empty($img_assetid)) {
				$asset->setAttrValue('html', NULL);
				return TRUE;
			}//end if

			$current_image = $am->getAsset($current_link['minorid']);
			$image_width = ($current_settings['fit_dimensions'] == TRUE) ? 'width: '.$current_settings['width'].'px;' : '';
			$image_height = ($current_settings['fit_dimensions'] == TRUE) ? 'height: '.$current_settings['height'].'px;' : '';
			$image_alt = $current_image->name;
			if ($current_image instanceof Image) {
				// Its an image, no problems here
				$image_alt = $current_image->attr('alt');
			} else {
				// Image Variety, so some work to be done to get alt
				$real_image = $am->getAsset($current_image->_imageid);
				$image_alt = $real_image->attr('alt');
			}
			$classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CONTENT_TYPE_IMAGE_CLASSES');

			// use the caption as requested from the original image or defined on the screen
			if ($current_settings['use_caption_attr']) {
				// Image variety do no have the caption  attribute, If asked to  use caption attr
				// of image request it from further up from the parent image of the image variety
				if (is_a($current_image, 'Image_Variety')) {
					$parent_image = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_image->_imageid);
					$caption = $parent_image->attr('caption');
				} else {
					$caption = $current_image->attr('caption');
				}
			} else {
				$caption = $current_settings['caption'];
			}

			$preview_classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CONTENT_TYPE_IMAGE_PREVIEW');
			ob_start();
				if ($on_click_option == 'inline') {
					// Print out js for showing the popup preview
					?>
					<div class="<?php echo $preview_classes['surrounding_container']; ?>" id="<?php echo $prefix; ?>_roll_preview" style="visibility: hidden; z-index: 5; position: absolute; left">
						<?php
						if ($current_settings['html_layout'] == 'table') { 
						?>
							<table class="<?php echo $preview_classes['preview_image_table']; ?>">
								<tr>
									<td class="<?php echo $preview_classes['close_preview_cell']; ?>" onclick="<?php echo $prefix; ?>_preview_close()" style="cursor: pointer;">
										Close
									</td>
								</tr>
								<tr>
									<td class="<?php echo $preview_classes['image_table_cell']; ?>">
										<img src="./?a=<?php echo $current_image->id; ?>" class="<?php echo $preview_classes['image_tag']; ?>" />
									</td>
								</tr>
							</table>
						<?php
						} else {
						?>
							<div class="<?php echo $preview_classes['close_preview_div']; ?>" onclick="<?php echo $prefix; ?>_preview_close()" style="cursor: pointer;">
								Close
							</div>
							<div class="<?php echo $preview_classes['image_div']; ?>">
								<img src="./?a=<?php echo $current_image->id; ?>" class="<?php echo $preview_classes['image_tag']; ?>" />
							</div>
						<?php
						}
						?>
					</div>

					<script type="text/javascript">
						/* Functions for Image Content Type: <?php echo $current_image->name; ?> */

						function <?php echo $prefix; ?>_find_posX()
						{
							obj = document.getElementById('<?php echo $prefix; ?>_img');
							var curleft = 0;
							if (obj.offsetParent) {
								while (obj.offsetParent) {
									curleft += obj.offsetLeft
									obj = obj.offsetParent;
								}
							} else if (obj.x) {
								curleft += obj.x;
							}
							return curleft;
						}

						function <?php echo $prefix; ?>_find_posY()
						{
							obj = document.getElementById('<?php echo $prefix; ?>_img');
							var curtop = 0;
							if (obj.offsetParent) {
								while (obj.offsetParent) {
									curtop += obj.offsetTop
									obj = obj.offsetParent;
								}
							} else if (obj.y) {
								curtop += obj.y;
							}
							return curtop;
						}

						function <?php echo $prefix; ?>_preview(evt)
						{
							var div = document.getElementById('<?php echo $prefix; ?>_roll_preview');
							if (div.style.visibility != 'hidden') return;
							var posx = 0;
							var posy = 0;
							posx = <?php echo $prefix; ?>_find_posX();
							posy = <?php echo $prefix; ?>_find_posY();
							div.style.left = posx + 'px';
							div.style.top = posy + 'px';
							div.style.visibility = '';
						}

						function <?php echo $prefix; ?>_preview_close()
						{
							var div = document.getElementById('<?php echo $prefix; ?>_roll_preview');
							div.style.visibility = 'hidden';
						}

					</script>
				<?php
				}//end if

				if ($current_settings['html_layout'] == 'table') { 
					?>
					<table class="<?php echo $classes['surrounding_container']; ?>">
						<?php
						if ($current_settings['caption_position'] == 'top') {
							?>
							<tr><td class="<?php echo $classes['caption_table_cell']; ?>"><?php echo $caption?></td></tr>
							<?php
						}
						?>
						<tr>
							<td class="<?php echo $classes['image_table_cell']; ?>" style="width:<?php echo $current_settings['width']; ?>px; height: <?php echo $current_settings['height']; ?>px;">
							<?php
							if ($image_linked) {
								?>
								<a href="./?a=<?php echo $current_image->id; ?>"
								<?php
									if ($on_click_option == 'new_window') {
										echo 'target="blank"';
									}
									?>
								>
								<?php
							}
							?>
							<img id="<?php echo $prefix; ?>_img" src="./?a=<?php echo $current_image->id; ?>" style="<?php echo $image_width.$image_height; ?>" class="<?php echo $classes['image_tag']; ?>" alt="<?php echo $image_alt; ?>"
								<?php
								// Add onclick events for inline previewing
								if ($on_click_option == 'inline') {
									echo 'onclick="return '.$prefix.'_preview(event);"';
								}
								?>
							/>
							<?php
							if ($image_linked) {
								?>
								</a>
								<?php
							}
							?>
							</td>
						</tr>
						<?php
						if ($current_settings['caption_position'] == 'bottom') {
							?>
							<tr><td class="<?php echo $classes['caption_table_cell']; ?>"><?php echo $caption ?></td></tr>
							<?php
						}
						?>
					</table>
					<?php
					} else {
					// we are using div html layout 
					?>
						<?php
						if ($current_settings['caption_position'] == 'top') {
						?>
							<div class="<?php echo $classes['caption_div']; ?>"><?php echo $caption ?></div>
						<?php
						}
						?>
						<div class="<?php echo $classes['image_div']; ?>" style="width:<?php echo $current_settings['width']; ?>px; height: <?php echo $current_settings['height']; ?>px;">
							<?php
							if ($image_linked) {
								?>
								<a href="./?a=<?php echo $current_image->id; ?>"
								<?php
									if ($on_click_option == 'new_window') {
										echo 'target="blank"';
									}
									?>
								>
								<?php
							}
							?>
							<img id="<?php echo $prefix; ?>_img" src="./?a=<?php echo $current_image->id; ?>" style="<?php echo $image_width.$image_height; ?>" class="<?php echo $classes['image_tag']; ?>" alt="<?php echo $image_alt; ?>"
								<?php
								// Add onclick events for inline previewing
								if ($on_click_option == 'inline') {
									echo 'onclick="return '.$prefix.'_preview(event);"';
								}
								?>
							/>
							<?php
							if ($image_linked) {
								?>
								</a>
								<?php
							}
							?>
						</div>
						<?php
						if ($current_settings['caption_position'] == 'bottom') {
						?>
							<div class="<?php echo $classes['caption_div']; ?>"><?php echo $caption ?></div>
						<?php
						}
						?>
					<?php
					}//end else
				$html = ob_get_contents();
			ob_end_clean();
			$asset->setAttrValue('html', $html);
		}//end if

		return TRUE;

	}//end processBackend()


}//end class

?>
