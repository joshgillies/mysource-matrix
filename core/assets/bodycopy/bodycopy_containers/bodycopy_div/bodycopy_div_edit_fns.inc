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
		$div_attributes['identifier'] = $asset->name;

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
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($designid);
					$classes_list = $design->attr('div_classes');
					if (!empty($classes_list)) {
						//$vars = explode("\n", $classes_list);
						foreach ($classes_list as $key => $value) {
							$classes[$key] = $value;
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
				$val = $div_attributes[$name];
				if ($val == '') continue;
				if ($name == 'css_class_list') continue;
				if ($name == 'dir') $dir_attribute = $div_attributes[$name];
				$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', FALSE, 'div_contents', 'major', TRUE);
		if (!empty($link)) {
			$content_type = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			$content_prefix = $prefix.'_'.get_class_lower($content_type).'_'.$link['linkid'];
		} else {
			$content_type = NULL;
			$content_prefix = '';
		}

		// require library functions needed for drawing the bodycopy divs
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');

		?>		
		<table<?php echo $attribute_list?> class="sq-<?php echo get_class_lower($content_type) ;?>-table" style="width:100%;">
			<tr>
				<td align="left" background="" id="<?php echo $prefix.'_div_'.$asset->id;?>" width="4%" style="border: 1px dashed #C0C0C0;">
				<span style="float: left;">
				<?php

				$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');

				$public_read = FALSE;
				if ($asset->status == SQ_STATUS_LIVE && $asset->readAccess(Array($public_userid))) {
					$public_read = TRUE;
				}
				$status_name = get_status_description($asset->status);
				$status_img = get_asset_status_icon($asset->status);
				$desc = translate('content_type_tooltip_status').': '.$status_img.'<b>'.get_status_description($asset->status).'</b><br />';
				$desc .= translate('content_type_tooltip_public').': <b>'.($public_read ? translate('yes') : translate('no')).'</b><br />';
				$desc .= (($popup_attribute_list) ?  translate('current_properties').':<br> '.$popup_attribute_list : '');
				Bodycopy_Edit_Fns::printBodycopyIcon('bodycopy_edit_div_properties(\''.$prefix.'\', '.$asset->id.', '.(($asset->status & SQ_SC_STATUS_SAFE_EDITING) ? 'false' : 'true').');', 'Edit '.$asset->attr('name').'\'s Properties', $desc, 'table_properties', '[Edit Properties]');
				if (!is_null($content_type)) {
					$content_edit = $content_type->getEditFns();
					$content_edit->paintBackendIcons($content_type, $content_prefix);
				}
				?></span><span style="float: right;"><?php				
				$lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset->id, 5);								
				foreach ($lineages as $lineage_key => $lineage) {
					if (!($lineage['link_type'] & SQ_SC_LINK_BACKEND_NAV)) {						
						unset($lineages[$lineage_key]);
					}
				}// end foreach
				?>				
				<?php
				if (count($lineages) > 1) {
					echo get_asset_tag_line_binoculars($asset->id, 'Multi-Linked DIV');
				}
				
				if (!is_null($content_type)) {
					$content_edit = $content_type->getEditFns();
					$content_edit->paintBackendStatusIcons($content_type, $content_prefix);
				}
				?>
				</span>
				</td>
			</tr>
			<tr>
				<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;" id="<?php echo $content_prefix; ?>_cell" class="sq-<?php echo get_class_lower($content_type) ;?>-wrapper">
				<?php
				if (!is_null($content_type)) {
					$content_edit = $content_type->getEditFns();
					if (method_exists($content_edit,'setDirAttr')) {
						$content_edit->setDirAttr($dir_attribute);
					}
					$content_edit->paintBackend($content_type, $content_prefix);
				}
				?>
				</td>
			</tr>
		</table>
		<?php

		return TRUE;

	}//end paintContainer()


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
