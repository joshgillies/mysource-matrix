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
* $Id: bodycopy_div_edit_fns.inc,v 1.51 2013/09/11 05:17:49 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc';

/**
* Bodycopy_Table_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.51 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Div_Edit_Fns extends Bodycopy_Container_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end __construct()


	/**
	* Prints the JavaScript needed by the backend functions for this div
	*
	* @param Bodycopy_Div	$asset	the div object being printed
	* @param string			$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	public function paintBackendJsData(Bodycopy_Div $asset, $prefix)
	{
		$div_attributes = $asset->attr('attributes');
		$div_attributes['identifier'] = $asset->attr('name');
		$div_attributes['desc'] = $asset->attr('description');

		?>
					case <?php echo $asset->id?> :

						retVal["attributes"] = '<?php echo var_serialise($div_attributes, TRUE)?>';

					break;
		<?php

	}//end paintBackendJsData()


	/**
	* Paint JS data that divs need to use to do their processing
	*
	* @param Bodycopy_Div		$bodycopy	the bodycopy being printed
	* @param Backend_Outputter	$o			the Backend Outputter instance
	* @param string				$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintGenericBackend(Asset $bodycopy, Backend_Outputter $o, $prefix)
	{
		// If the parent bodycopy class is itself, fudge the containers array
		// so that the proper stuff gets drawn when putting out the javascript
		// to edit.
		if ($bodycopy instanceof Bodycopy_Div) {
			$containers = Array(Array('minorid'=>$bodycopy->id, 'minor_type_code'=>get_class_lower($bodycopy)));
		} else {
			$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		}

		?>
		<script language="JavaScript" src="<?php echo sq_web_path('data').'/asset_types/bodycopy/js/bodycopy_edit_divs.js'?>"></script>

		<script language="JavaScript" type="text/javascript">
		var bodycopy_available_conditions = {};
		var bodycopy_applied_template = {};
		function serialise_div(bodycopy_name, bodycopy_data, divid) {
			var form = document.main_form;
			if (!bodycopy_saved[bodycopy_name] && bodycopy_name != null) bodycopy_saved[bodycopy_name] = new Object();

			if (divid != null) {
				if (!bodycopy_saved[bodycopy_name][divid]) {
					bodycopy_saved[bodycopy_name][divid] = new Object();
				}

				bodycopy_saved[bodycopy_name][divid]['attributes'] = bodycopy_data['attributes'];
			}// end if divid

			form.elements['bodycopy_saved[' + bodycopy_name + '][' + divid + ']'].value = var_serialise(bodycopy_saved[bodycopy_name][divid]);
		}// end serialise_div()

		// general fn that the generic include fns can use to get data to use
		function get_bodycopy_current_div_data(bodycopy_name, divid) {
			if (divid == null) divid = -1;
			return bodycopy_current_data[bodycopy_name].get_div_data(divid);
		}

		// general function that the generic include fns can use to get data to use ()
		// used in getting numebr of assets the DIV is linked to
		function get_bodycopy_current_div_count(bodycopy_name, asset_linked) {
			return bodycopy_current_data[bodycopy_name].get_div_data(linked);
		}

		function get_bodycopy_div_available_classes() {
			<?php
			$classes = Array();
			if (isset($GLOBALS['sq_preview_url']) && !empty($GLOBALS['sq_preview_url'])) {
				$url = $GLOBALS['sq_preview_url'];
				$url = preg_replace('|^http[s]?://|', '', $url);

				$designid = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, 'design::%frontend%', TRUE);

				if ($designid) {
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($designid, '', TRUE);
					if(!empty($design)) {
						$classes_list = $design->attr('div_classes');
						if (!empty($classes_list)) {
							foreach ($classes_list as $friendly_name => $value) {
								$class_names = array_get_index($value, 'classNames', $friendly_name);
								$classes[$class_names] = $friendly_name;
							}
						}
					}
				}
			}
			?>
			return var_unserialise('<?php echo empty($classes) ? NULL : var_serialise($classes, TRUE); ?>');
		}

		function get_bodycopy_<?php echo $prefix?>_current_div_data(divid) {

			retVal = new Object();

			switch (divid) {
		<?php
		// if we have tables get them to print the js data they need
		$i = 0;
		if (count($containers)) {
			foreach ($containers as $container_data) {
				if ($container_data['minor_type_code'] != 'bodycopy_div') {
					continue;
				}

				$div = $GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
				$div_edit = $div->getEditFns();
				$div_edit->paintBackendJsData($div, $prefix, $i);
				$i++;
			}
		}
		?>
				default :
					retVal["num_containers"] = <?php echo count($containers); ?>;

			}// end switch divid


			// check if this piece of data has been changed before
			// if it has, return the changed version, not the original
			if (divid != -1) {
				if (bodycopy_data_exists(new Array('<?php echo $prefix?>', divid, 'attributes'))) {
					retVal ["attributes"] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][divid]["attributes"]);
					return retVal;
				}
			}

			return retVal;
		}// end get_bodycopy_<?php echo $prefix?>_current_table_data()

		// set reference so generic fn can be called above
		bodycopy_current_data["<?php echo $prefix?>"].get_div_data = get_bodycopy_<?php echo $prefix?>_current_div_data;
		</script>

		<?php

	}//end paintGenericBackend()


	/**
	* Paints this div as HTML
	*
	* @param Bodycopy_Div	$asset		the div whose interface we are painting
	* @param boolean		$editing	are we printing an editing interface
	* @param boolean		$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	public function paint(Bodycopy_Div $asset, $editing=FALSE, $generating=FALSE)
	{

		$div_attributes = $asset->attr('attributes');
		if (!isset($div_attributes['layout_type'])) {
			$div_attributes['layout_type'] = 'div';
		}

		$attribute_list = '';

		for (reset($div_attributes); $name = key($div_attributes); next($div_attributes)) {
			$val = $div_attributes[$name];
			if ($val == '') continue;

			$skip_attribute = FALSE;
			switch ($name) {
				case 'layout_type':
				case 'content_type':
				case 'identifier':
				case 'disable_keywords':
				case 'css_class_list':
					$skip_attribute = TRUE;
				break;
				case 'css_class':
					$name = 'class';
				break;
			    	case 'conditions':
					$skip_attribute = TRUE;
				break;
				case 'template':
					$skip_attribute = TRUE;
				break;
			}

			if ($skip_attribute) continue;
			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
		}

		// Now add the ID of the DIV
		$val = clean_div_attribute($asset->attr('name'));
		if (!empty($val)) {
			$attribute_list .= ' '.'id="'.$val.'"';
		}

		// print DIV or SPAN tag if we are not printing raw HTML
		if ($div_attributes['layout_type'] != 'none') {
			//check if content type exists and then make a class out of it, if not, just add default
			$content_type_class = 'default';
			if(isset($div_attributes['content_type'])){
				$content_type_class = $div_attributes['content_type'];
			}
			echo "\n<".strtolower($div_attributes['layout_type']).$attribute_list.">\n";
		}


		$content_type = $asset->getContentType();

		if (!is_null($content_type)) {
			$content_edit = $content_type->getEditFns();
			$content_edit->paint($content_type, $editing, $generating);
		}

		// print end DIV or SPAN tag if we are not printing raw HTML
		if ($div_attributes['layout_type'] != 'none') {
			echo "\n</".strtolower($div_attributes['layout_type']).">\n";
		}

		return TRUE;

	}//end paint()


	/**
	* Paint the interface for editing this div
	*
	* @param Bodycopy_Div		$asset	the div whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter instance
	* @param string				$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintContainer(Bodycopy_Div $asset, Backend_Outputter $o, $prefix)
	{
		if (!parent::paintContainer($asset, $o, $prefix)) {
			return FALSE;
		}

		$div_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$popup_attribute_list = '';
		$dir_attribute = '';

		if (!empty($div_attributes)) {
			for (reset($div_attributes); $name = key($div_attributes); next($div_attributes)) {
				if ($name === 'desc') {
					$val = $asset->attr('description');
				} else {
					$val = $div_attributes[$name];
				}
				if ($val == '') continue;
				if ($name == 'css_class_list') continue;
				if ($name == 'conditions') continue;
				if ($name == 'dir') $dir_attribute = $div_attributes[$name];
				$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"<br/>';
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', FALSE, 'div_contents', 'major', TRUE);
		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			$content_prefix = $prefix.'_'.get_class_lower($content_type).'_'.$link['linkid'];
			$content_type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(get_class_lower($content_type), 'name');
			//Create a friendly name for the content type name

			$cc_suffix = ' Content Type';
			if (substr($content_type_name, -strlen($cc_suffix)) === $cc_suffix) {
				$content_type_name = sprintf(
					translate('%s Content Container'),
					substr($content_type_name, 0, -strlen($cc_suffix))
				);
			}

			$nc_prefix = 'Nest Content ';
			if (substr($content_type_name, 0, strlen($nc_prefix)) === $nc_prefix) {
				$content_type_name = sprintf(
					translate('Nested Content %s'),
					substr($content_type_name, strlen($nc_prefix))
				);
			}
		} else {
			$content_type = NULL;
			$content_prefix = '';
			$content_type_name = '';
		}

		// require library functions needed for drawing the bodycopy divs
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');

		if ($content_type->deprecated === TRUE) {
			$GLOBALS['SQ_SYSTEM']->printDeprecatedWarning($content_type);
		}

		// work out applied container template
		// and then save it in JS var (for backend div property popup) and data attribute (for edit+)
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'minor', 'container_template');
		$applied_template = '';
		$applied_template_name = '';
		if(!empty($root_links[0]['majorid'])) {
			$applied_template = $root_links[0]['majorid'];
			$applied_template_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($applied_template);
			$applied_template_name = $applied_template_asset->attr('name');
		}



		// Work out the conditions we have available for this container,
		// and whether we have conditional display active.
		$all_conditions            = Array();
		$is_conditional_display    = FALSE;
		$is_conditions_set = isset($div_attributes['conditions']['conditions']) && !empty($div_attributes['conditions']['conditions']);
		$dependant_parent_assetids = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id, '', FALSE, FALSE);
		foreach ($dependant_parent_assetids as $parentid) {
			$dependant_parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
			// get all available conditions
			$available_conditions = array_keys($dependant_parent_asset->getAvailableConditions());
			if (!empty($available_conditions)) {
			$all_conditions = array_merge($all_conditions, $available_conditions);
			}
			// test if there is set conditions
			$conditions = $dependant_parent_asset->getConditions();
			if (isset($conditions['conditional_div']['div_'.$asset->id]['conditions'][0]['conditions'][0])) {
				$is_conditional_display = TRUE;
			}
		}

		// Build condition JS if there are conditions to use.
		if (!empty($all_conditions)) {
			$conditions_js = json_encode_array(array_unique($all_conditions));
		}

		$condition_class = '';
		if($is_conditional_display) {
		    $condition_class .= ' div-conditions-set';
		}
		else {
		    if($is_conditions_set) {
			$condition_class .= ' div-conditions-set-disabled';
		    }
		}

		?>
		<table<?php echo $attribute_list?> class="sq-edit-bodycopy-section sq-<?php echo get_class_lower($content_type) ;?>-table">
			<tr>
				<td align="left" background="" id="<?php echo $prefix.'_div_'.$asset->id;?>" class="sq-grey-header-wrapper<?php echo $condition_class;?>"<?php
				   if (!empty($all_conditions)) {
					   ?> data-available-conditions="<?php echo htmlspecialchars($conditions_js); ?>"<?php
				   }
				   if (!empty($applied_template)) {
					   ?> data-applied-template="<?php echo htmlspecialchars($applied_template); ?>"<?php
					   ?> data-applied-template-name="<?php echo htmlspecialchars($applied_template_name); ?>"<?php
				   }
				?>>
					<div class="sq-grey-header-row clearfix">

						<span class="sq-icon-wrapper left">
							<?php
							$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

							$public_read = FALSE;
							if ($asset->status == SQ_STATUS_LIVE && $asset->readAccess(Array($public_userid))) {
								$public_read = TRUE;
							}
							$status_name = get_status_description($asset->status);
							$status_img = get_asset_status_icon($asset->status);
							$desc = '<table class="sq-toolbox-properties-table">';
							$desc .= '<tr><td>'.translate('Status').'</td><td>'.$status_img.''.get_status_description($asset->status).'</td><tr>';

							$desc .= '<tr><td>'.translate('Public').'</td><td>'.($public_read ? translate('Yes') : translate('No')).'</td><tr>';



							$desc .= '<tr><td>'.(($popup_attribute_list) ?  translate('Current Properties').'</td><td> '.$popup_attribute_list :''.'</td><tr>');

							$desc .= '</table>';
							Bodycopy_Edit_Fns::printBodycopyIcon(
									'bodycopy_edit_div_properties(\''.$prefix.'\', '.$asset->id.', '.(($asset->status & SQ_SC_STATUS_SAFE_EDITING) ? 'false' : 'true').');',
									sprintf(translate('Edit %s\'s Properties'), $asset->attr('name')),
									$desc,
									'table_properties',
									'['.translate('Edit Properties').']',
									'class="sq-icon"');

							// print condition editing icon if there are conditions to use
							if(!empty($all_conditions)) {
							    	$condition_count = count($all_conditions);
								$desc = sprintf(translate_plural(
									'There is 1 available condition to set.',
									'There are %s available conditions to set.',
									$condition_count
								), $condition_count);
								?><script language="JavaScript" type="text/javascript">bodycopy_available_conditions['<?php echo $asset->id; ?>'] = <?php echo $conditions_js; ?>; </script><?php
								Bodycopy_Edit_Fns::printBodycopyIcon(
									'bodycopy_edit_div_conditions(\''.$prefix.'\', '.$asset->id.', '.$parentid.');',
									translate('Edit Container Display Conditions'),
									$desc,
									'conditions',
									'['.translate('Edit Container Display Conditions').']'
								);
							}

							if(!empty($applied_template)) {
											?><script language="JavaScript" type="text/javascript">bodycopy_applied_template['<?php echo $asset->id; ?>'] = <?php echo $applied_template; ?>; </script><?php
							}

							if (!is_null($content_type)) {
								$content_edit = $content_type->getEditFns();
								$content_edit->paintBackendIcons($content_type, $content_prefix, $o);
							}
							?>
						</span>
						<span class="sq-icon-wrapper right">
							<?php
							// print the condition set icon
							if($is_conditional_display) {
								Bodycopy_Edit_Fns::printBodycopyIcon(
									'', 
									translate('Conditionally Displayed'), 
									translate('This container has display conditions set on it.'),
									'conditions_set',
									'['.translate('Conditional Display').']'
								);
							}

							$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset->id, 5);
							foreach ($lineages as $lineage_key => $lineage) {
								if (!($lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV)) {
									unset($lineages[$lineage_key]);
								}
							}// end foreach
							?>
							<?php
							if (count($lineages) > 1) {
								echo get_asset_tag_line_binoculars($asset->id, translate('Multi-Linked DIV'));
							}

							if (!is_null($content_type)) {
								$content_edit = $content_type->getEditFns();
								$content_edit->paintBackendStatusIcons($content_type, $content_prefix, $o);
							}
							?>
							<span class="sq-status-square status-<?php echo str_replace(' ', '-', strtolower(get_status_description($asset->status))) ;?>" title="<?php echo $status_name ;?>"></span>
						</span>
						<strong title="<?php echo $content_type_name ;?> - Asset Id: <?php echo $asset->id ;?> - <?php echo $status_name ;?>" class="sq-content-type-name"><?php echo $asset->name ;?> <?php if(!empty($applied_template)) { echo ' - '.$applied_template_name; } ?></strong>
						<?php
							if ($content_type instanceof Content_Type_Nest_Content) {
								// the toggle link to show/hide the extra bits for a nested container to make the UI look simple and easy to ready
								// see RM #6067 Improve the layout and UI of the nested content container
								?> - <a href="javascript:void(0);" onclick="toggleNestedContents<?php echo $content_type->id; ?>(); return false"><?php echo translate('Toggle Additional Options'); ?></a>
						<?php
							}
						?>
					</div>
				</td>
			</tr>
			<tr>
				<td id="<?php echo $content_prefix; ?>_cell" class="sq-content-type-wrapper">
				<?php
				if (!is_null($content_type)) {
					$content_edit = $content_type->getEditFns();
					if (method_exists($content_edit,'setDirAttr')) {
						$content_edit->setDirAttr($dir_attribute);
					}
					?>
					<div class="sq-edit-content-type-wrapper">
						<?php

						// we have custom template applied
						$use_template = $this->paintCustomLayoutInterface($asset, $applied_template);
						if(!$use_template) {
							// or 
							$content_edit->paintBackend($content_type, $content_prefix, $o, $asset);
						}
						?>
					</div>
					<?php
				}
				?>
				</td>
			</tr>
		</table>
		<?php

		return TRUE;

	}//end paintContainer()


	/**
	* Paint the interface for custom template
	*
	* @param Bodycopy_Div		$asset				the div whose interface we are painting
	* @param string	 			$applied_template	the applied template ID
	* @param boolean	 		$edtable			should we make metadata fields editable
	*
	* @return boolean
	* @access public
	*/
	public function paintCustomLayoutInterface(Bodycopy_Div $asset, $applied_template, $edtable=TRUE)
	{
			if(empty($applied_template)) return FALSE;
			// if we have containe template applied, we will need to paint the container with template
			$template = !empty($applied_template) ? $GLOBALS['SQ_SYSTEM']->am->getAsset($applied_template) : NULL;
			if(empty($template)) return FALSE;

			// we are in backend and template is not enabled in backend, don't worry about it
			$enable_edit_layout_in_backend = $template->attr('edit_interface_in_admin');
			if(SQ_IN_BACKEND && !$enable_edit_layout_in_backend) return FALSE;

			$layout = 'default_layout';
			$links = $template->getSimpleEditLayoutLinks();
			if(isset($links[0]['minorid'])) {
				$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($links[0]['minorid']);
			}


			// acquire metadata locks
			if(empty($this->_tmp['metadata_lock'])) {
				if($edtable) {
					$locks = $GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'metadata');
				}
				else {
					$locks = FALSE;
					if($asset->writeAccess('metadata')) {
						$locks = $GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'metadata');
					}
				}
				$this->_tmp['metadata_lock'] = $locks;
			}

			echo '<div class="content_template_custom_layout">';
			$owner = $asset;
			$aei = new Asset_Edit_Interface($owner->type());
			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			if(SQ_IN_BACKEND) {
				$o =  new Backend_Outputter();
			}
			else if(SQ_IN_LIMBO){
				$o =  new Limbo_Outputter();
			}
			else {
				$o =  new Limbo_Outputter();
				$GLOBALS['SQ_SYSTEM']->backend->out = new Limbo_Outputter();
			}
			// mark the current container id, so we can use its prefix later when painting metadata fields
			$GLOBALS['SQ_CURRENT_BODYCOPY_CONTAINER'] = $asset->id;
			if($layout === 'default_layout') {
				// paint the default contents screen first
				$replacement = $asset->getCustomKeywordReplacements(Array('contents'));
				if(isset($replacement['replacements']['contents'])) {
					echo $replacement['replacements']['contents'];
				}
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$schemas = $mm->getSchemas($owner->id);
				if(!empty($schemas)) {
					// paint the whole metadata value screen
					$o->setBuffering(FALSE);
					$keywords['fields']['metadata_values'] = 'metadata_values';
					$aei->includeStaticScreen('metadata');
					ob_start();
					paintMetadata($owner, $o, $aei, $keywords, Array('show_cascade_metadata_field' => FALSE));
					$contents = ob_get_contents();
					ob_end_clean();
					echo($aei->_tmp['layout_keyword_replacements']['metadata_values']);
				}
			}
			else {
				$layout_keywords_order = $aei->_getLayoutKeywordsOrder($layout);
				$aei->getKeywordsReplacementsForPaint($owner, $o, $layout, $layout_keywords_order, TRUE, FALSE, '');
				$layout->printBody();
			}
			// include javascripts needed by those metadata fields
			if(!($o instanceof Limbo_Outputter)) {
				foreach ($o->_js_includes as $file) {
					$o->_paintJsInclude($file);
				}
			}
			echo '</div>';

			return TRUE;
	}//end paintCustomLayoutInterface

	/**
	* Process the interface for editing this div
	*
	* @param Bodycopy_Div		$asset		the div whose interface we are painting
	* @param Backend_Outputter	$o			the Backend Outputter instance
	* @param string				$prefix		prefix for the form element
	* @param string				$bc_action	a global bodycopy action to perform (eg insert_table_row)
	* @param string				$bc_name	the name of the bodycopy that submitted the action
	* @param array				$bc_data	an array of data needed to perform the action
	* @param array				$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	public function processContainer(Bodycopy_Div $asset, Backend_Outputter $o, $prefix, $bc_action, $bc_name, Array $bc_data, Array $bc_saved)
	{

		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'minor', 'container_template');
		// we have custom template applied
		if(isset($root_links[0]['majorid'])) {
			$owner = $asset;
			$aei = new Asset_Edit_Interface($owner->type());
			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			$o =  new Limbo_Outputter();
			$aei->includeStaticScreen('metadata');
			$GLOBALS['SQ_CURRENT_BODYCOPY_CONTAINER'] = $asset->id;
			if (processMetadata($owner, $o, $this)) {
				$ret_val = TRUE;
			}
		}

		$updated = parent::processContainer($asset, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved);

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', FALSE, 'div_contents', 'major', TRUE);

		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!is_null($content_type)) {
				$content_prefix = $prefix.'_'.get_class_lower($content_type).'_'.$link['linkid'];
				$content_edit = $content_type->getEditFns();
				if ($content_edit->processBackend(Array(), $content_type, $content_prefix)) {
					if ($content_type->saveAttributes()) $updated = TRUE;
				}
			}
		}

		if (isset($bc_saved[$prefix][$asset->id]['attributes']['content_type'])) {
			if ($this->_updateCellType($asset, $bc_saved[$prefix][$asset->id]['attributes']['content_type'])) {
				$updated = TRUE;
			}
		}

		if ($updated) $this->generateContentFile($asset);
		$this->_contentsUpdated($asset);
		return $updated;

	}//end processContainer()


	/**
	* Change the type of this DIV's editing interface to the passed content type
	*
	* @param Bodycopy_Div	$asset		the DIV object
	* @param string			$new_type	the name of the new content type
	*
	* @return boolean
	* @access private
	*/
	protected function _updateCellType(Bodycopy_Div $asset, $new_type='')
	{
		// dont change anything if the new type id blank
		$new_type = trim($new_type);
		if (empty($new_type)) return FALSE;

		// get the contentType
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', FALSE, 'div_contents', 'major', TRUE);
		if (empty($link)) return FALSE;
		$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

		// if the new type is the same as the current, change nothing
		$current_type = get_class_lower($content_type);
		if ($current_type == $new_type) return FALSE;

		if (method_exists($content_type, 'validateContent') && !$content_type->validateContent($new_type)) {
			return FALSE;
		}

		// morph the content type to the new type
		// first up
		$morphed = $content_type->morph('content_type');
		if (!$morphed) return FALSE;

		// then down
		$content_type = $morphed->morph($new_type);
		if (!$content_type) return FALSE;

		return TRUE;

	}//end _updateCellType()


}//end class

?>
