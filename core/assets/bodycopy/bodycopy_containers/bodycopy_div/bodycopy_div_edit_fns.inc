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
* $Id: bodycopy_div_edit_fns.inc,v 1.11 2004/07/19 00:46:14 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container_edit_fns.inc';

/**
* Bodycopy_Table_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Div_Edit_Fns extends Bodycopy_Container_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bodycopy_Div_Edit_Fns()
	{
		$this->Bodycopy_Container_Edit_Fns();

	}//end constructor


	/**
	* Prints the JavaScript needed by the backend functions for this div
	*
	* @param object Bodycopy_Div	&$asset	the div object being printed
	* @param string					$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	function paintBackendJsData(&$asset, $prefix)
	{
		$div_attributes = $asset->attr('attributes');
		$div_attributes['identifier'] = $asset->name;

		?>
					case <?php echo $asset->id?> :

						retVal["attributes"] = '<?php echo var_serialise($div_attributes, true)?>';

					break;
		<?php
	
	}//end paintBackendJsData()


	/**
	* Paint JS data that divs need to use to do their processing
	*
	* @param object Bodycopy			&$bodycopy	the bodycopy being printed
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintGenericBackend(&$bodycopy, &$o, $prefix)
	{
		// If the parent bodycopy class is itself, fudge the containers array
		// so that the proper stuff gets drawn when putting out the javascript
		// to edit.
		if (is_a($bodycopy, 'bodycopy_div')) {
			$containers = Array(Array('minorid'=>$bodycopy->id, 'minor_type_code'=>'bodycopy_div'));
		} else {
			$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
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

		function get_bodycopy_<?php echo $prefix?>_current_div_data(divid) {

			retVal = new Object();
			
			switch (divid) {
		<?php
		// if we have tables get them to print the js data they need
		$i = 0;
		if (count($containers)) {
			foreach ($containers as $container_data) {
				if ($container_data['minor_type_code'] != 'bodycopy_div') continue;
				$div = &$GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
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
	* @param object Bodycopy_Div	&$asset		the div whose interface we are painting
	* @param booelan				$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $generating=false)
	{
		$div_attributes = $asset->attr('attributes');
		if (!isset($div_attributes['layout_type'])) $div_attributes['layout_type'] = 'div';

		$attribute_list = '';

		for (reset($div_attributes); $name = key($div_attributes); next($div_attributes)) {
			if ($name == 'layout_type')  continue;
			if ($name == 'content_type') continue;
			if ($name == 'identifier')   continue;

			$val = $div_attributes[$name];
			if ($val == '') continue;
			if ($name == 'css_class') $name = 'class';

			$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
		}

		// Now add the ID of the DIV
		$val = str_replace(' ', '_', strtolower($asset->attr('name')));
		$attribute_list .= ' '.'id="'.str_replace('"', '&quot;', $val).'"';

		// print DIV or SPAN tag if we are not printing raw HTML
		if ($div_attributes['layout_type'] != 'none') echo "\n<".strtolower($div_attributes['layout_type']).$attribute_list.">\n";

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', false, 'div_contents', 'major', true);

		if (!empty($link)) {
			$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($content_type)) {
				echo 'Content Type Asset Id : #'.$link['minorid'].' not found<br>Please contact your system administrator';
			} else {
				$content_edit = &$content_type->getEditFns();
				$content_prefix = $asset->getPrefix().'_'.get_class($content_type).'_'.$link['linkid'];
				$content_edit->paint($content_type, $content_prefix, $generating);
			}
		}

		// print end DIV or SPAN tag if we are not printing raw HTML
		if ($div_attributes['layout_type'] != 'none') echo "\n</".strtolower($div_attributes['layout_type']).">\n";

		return true;

	}//end paint()


	/**
	* Paint the interface for editing this div
	*
	* @param object Bodycopy_Div		&$asset	the div whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintContainer(&$asset, &$o, $prefix)
	{
		if (!parent::paintContainer($asset, $o, $prefix)) return false;
		
		$div_attributes = $asset->attr('attributes');

		$attribute_list = '';
		$popup_attribute_list = '';

		if (!empty($div_attributes)) {
			for (reset($div_attributes); $name = key($div_attributes); next($div_attributes)) {
				$val = $div_attributes[$name];
				if ($val == '') continue;
				$popup_attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
				$attribute_list .= ' '.$name.'="'.str_replace('"', '&quot;', $val).'"';
			}
		}

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', false, 'div_contents', 'major', true);
		if (!empty($link)) {
			$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			$content_prefix = $prefix.'_'.get_class($content_type).'_'.$link['linkid'];
		} else {
			$content_type = null;
			$content_prefix = '';
		}
		
		// require library functions needed for drawing the bodycopy divs.
		$type_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo('bodycopy');
		require_once SQ_SYSTEM_ROOT.'/'.$type_info['dir'].'/files/lib.php';

		?>
		<table<?php echo $attribute_list?> style="width:100%;">
			<tr>
				<td align="left" background="" id="<?php echo $prefix.'_div_'.$asset->id;?>" width="4%" style="border: 1px dashed #C0C0C0;">
				<?php
				print_bodycopy_icon("Javascript: bodycopy_edit_div_properties('$prefix', $asset->id, ".(($asset->status & SQ_SC_STATUS_SAFE_EDITING) ? 'false' : 'true').");", 'Edit '.$asset->attr('name').'\'s Properties', (($popup_attribute_list) ? 'Current Properties :<br>'.$popup_attribute_list : ''), 'table_properties');
				
				if (!is_null($content_type)) {
					$content_edit = &$content_type->getEditFns();
					$content_edit->paintBackendIcons($content_type, $content_prefix);
				}
				?>
				</td>
			</tr>
			<tr>
				<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;" id="<?php echo $content_prefix; ?>_cell">
				<?php
				if (!is_null($content_type)) {
					$content_edit = &$content_type->getEditFns();
					$content_edit->paintBackend($content_type, $content_prefix);
				}
				?>
				</td>
			</tr>
		</table>
		<?php

		return true;

	}//end paintContainer()


	/**
	* Process the interface for editing this div
	*
	* @param object BodyCopy_Div		&$asset		the div whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form element
	* @param string						$bc_action	a global bodycopy action to perform (eg insert_table_row)
	* @param string						$bc_name	the name of the bodycopy that submitted the action
	* @param array						$bc_data	an array of data needed to perform the action
	* @param array						$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	function processContainer(&$asset, &$o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)
	{
		$updated = parent::processContainer($asset, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved);

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', false, 'div_contents', 'major', true);

		if (!empty($link)) {
			$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!is_null($content_type)) {
				$content_prefix = $prefix.'_'.get_class($content_type).'_'.$link['linkid'];
				$content_edit = &$content_type->getEditFns();
				if ($content_edit->processBackend(Array(), $content_type, $content_prefix)) {
					if ($content_type->saveAttributes()) $updated = true;
				}
			}
		}

		if (isset($bc_saved[$prefix][$asset->id]['attributes']['content_type'])) {
			if ($this->_updateCellType($asset, $bc_saved[$prefix][$asset->id]['attributes']['content_type'])) $updated = true;
		}

		if ($updated) $this->generateContentFile($asset);
		$this->_contentsUpdated($asset);
		return $updated;

	}//end processContainer()


	/**
	* Change the type of this DIV's editing interface to the passed content type
	*
	* @param object BodyCopy_DIV	&$asset		the DIV object
	* @param string					$new_type	the name of the new content type
	*
	* @return boolean
	* @access private
	*/
	function _updateCellType(&$asset, $new_type='') {

		// dont change anything if the new type id blank
		$new_type = trim($new_type);
		if (empty($new_type)) return false;

		// get the contentType
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'content_type', false, 'div_contents', 'major', true);
		if (empty($link)) return false;
		$content_type = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

		// if the new type is the same as the current, change nothing
		$current_type = get_class($content_type);
		if ($current_type == $new_type) return false;

		// morph the content type to the new type
		// first up
		if (!$content_type->morph('content_type')) return false;
		// then down
		if (!$content_type->morph($new_type)) return false;

		return true;

	}//end _updateCellType()
	

}//end class

?>
