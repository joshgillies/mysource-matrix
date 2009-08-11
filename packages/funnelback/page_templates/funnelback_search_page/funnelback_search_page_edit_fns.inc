<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: funnelback_search_page_edit_fns.inc,v 1.1.1.1 2009/08/11 01:51:56 bpearson Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Funnelback_Search_Page_Edit_Fns
*
*     Edit functions for funnelback search page
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Search_Page_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'content';
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint links to the default formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'initial', translate('funnelback_initial_bodycopy_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'results', translate('funnelback_results_bodycopy_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('funnelback_no_results_bodycopy_desc'));

			$folder = $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('funnelback_default_bodycopy_desc'));
			$this->_paintDefaultFormatLink($folder->id, 'featured_format', translate('funnelback_featured_bodycopy_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Paint links to the fluster formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFlusterFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
			<?php
			$folder = $asset->getFolder('fluster_formats');
			$this->_paintDefaultFormatLink($folder->id, 'fluster_format', translate('funnelback_fluster_format_desc'));
			$this->_paintDefaultFormatLink($folder->id, 'cluster_format', translate('funnelback_cluster_format_desc'));
			$this->_paintDefaultFormatLink($folder->id, 'navigation_format', translate('funnelback_navigation_format_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintFlusterFormatLinks()


	/**
	* Paint links to the spell formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSpellFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('edit_format') ?></th>
				<th><?php echo translate('note') ?></th>
			</tr>
			<?php
			$folder = $asset->getFolder('spell_formats');
			$this->_paintDefaultFormatLink($folder->id, 'spell_format', translate('funnelback_spell_format_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintSpellFormatLinks()


	/**
	* Prints the table containing the links to the bodycopies (and default type format)
	*
	* @param string	$root_assetid			the id of the asset to look under for bodycopies; NULL indicates the listing_engine asset itself
	* @param string		$bodycopy_name			the name of the bodycopy to print, eg. 'no_results'
	* @param string		$bodycopy_description	the description to label the bodycopy with
	* @param boolean	$bodycopy_description	print the delete control or not
	* @param string		$prefix					the prefix of the asset
	* @param string		$type_code				the type_code of the format to the print (by default, 'bodycopy')
	* @param string		$screen					the edit screen to link to
	*
	* @return void
	* @access private
	*/
	function _paintDefaultFormatLink($root_assetid, $bodycopy_name, $bodycopy_description, $delete_control=FALSE, $prefix='', $type_code='bodycopy', $screen='contents')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($root_assetid, SQ_LINK_TYPE_2, $type_code, TRUE, $bodycopy_name);
		if (!empty($link)) {
			$info = Array($link['minorid'] => 'contents');
			$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
			?>
			<tr>
				<td>
					<?php echo get_asset_tag_line($link['minorid'], $screen); ?>
				</td>
				<td>
					<?php echo $bodycopy_description; ?>
				</td>
				<?php
				if ($delete_control) {
					echo '<td>';
					check_box($prefix.'_delete['.$bodycopy_name.']');
					echo '</td>';	
				}//end if
				?>
			</tr>
			<?php
		}//end if

	}//end _paintDefaultFormatLink()


	/**
	* Paint the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTypeFormats(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$folder = $asset->getFolder('type_formats');
		$formats = $asset->getFormats('type_formats');

		if (count($formats) > 2) {
			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('edit_format') ?></th>
					<th><?php echo translate('note') ?></th>
					<?php
					if ($write_access) {
						?>
						<th><?php echo translate('delete_question') ?></th>
						<?php
					}
					?>
				</tr>
				<?php
				foreach ($formats as $minorid => $value) {
					if ($value == 'default_format') continue;
					if ($value == 'featured_format') continue;

					$this->_paintDefaultFormatLink($folder->id, $value, translate('funnelback_custom_format_desc', strtoupper($value)), $write_access, $prefix);
				}//end foreach
				?>
			</table>
			<?php
		} else {
			echo translate('funnelback_no_type_formats');
		}//end if cutomised formats

		if ($write_access) {
			echo '<p><b>'.translate('funnelback_new_type_format').'</b> ';
			text_box($prefix.'_new_type', '');
			echo '</p>';
		}
		$o->note(translate('funnelback_custom_format_note'));
		return TRUE;

	}//end paintTypeFormats()


	/**
	* Process the interface for editing custom type formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTypeFormats(&$asset, &$o, $prefix)
	{
		$folder = $asset->getFolder('type_formats');

		$new_type = array_get_index($_POST, $prefix.'_new_type', '');
		$deleted_formats = array_get_index($_POST, $prefix.'_delete', Array());

		// User wants to delete and create the same one? No point wasting time, just leave it and move on
		if (isset($deleted_formats[$new_type])) {
			unset($deleted_formats[$new_type]);
			$new_type = '';
		}//end if
	
		foreach ($deleted_formats as $type => $on) {
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $type);
			if (!empty($current_link)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_link['linkid'])) {
					trigger_localised_error('FNB0006', E_USER_WARNING, $type);
				}//end if
			}//end if
		}//end foreach
	

		if (!empty($new_type)) {
			// check that this type isnt already customised
			$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $new_type);
			if (empty($current_link)) {
				// create a new format bodycopy for this type
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
	
				$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
	
				$bodycopy = new Bodycopy();
				$bodycopy->setAttrValue('name', strtoupper($new_type).' Format');
				$copy_args = Array('content' => $asset->_getDefaultBodycopyContent('custom_format'));
	
				if (!$bodycopy->create($copy_link, $copy_args)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}//end if
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			} else {
				trigger_localised_error('FNB0005', E_USER_WARNING, $new_type);
			}//end if
		}//end if

		return FALSE;

	}//end processTypeFormats()


	/**
	* Paints the default scope options
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultScope(&$asset, &$o, $prefix)
	{
		$scope   = $asset->attr('scope');
		$default = $asset->attr('default_scope');

		if ($asset->writeAccess('attributes')) {
			echo combo_box($prefix.'_default_scope', $scope, FALSE, $default);
		} else {
			echo (isset($scope[$default])) ? $scope[$default] : translate('funnelback_no_default_scope');
		}//end if

		return TRUE;

	}//end paintDefaultScope()


	/**
	* Processes the default scope options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processDefaultScope(&$asset, &$o, $prefix)
	{
		// Save the values
		$processed_value = array_get_index($_POST, $prefix.'_default_scope', '');
		if (!empty($processed_value) && $asset->writeAccess('attributes')) {
			$asset->setAttrValue('default_scope', $processed_value);
			$asset->saveAttributes();
		}//end if

		return TRUE;

	}//end processDefaultScope()


	/**
	* Paints the scope options
	*
	* @param object		&$asset		asset being painted
	* @param object		&$o			backend outputter
	* @param string		$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintScope(&$asset, &$o, $prefix)
	{
		$scope = $asset->attr('scope');

		$this->_paintOptionValue($scope, $asset, $o, $prefix);

		return TRUE;

	}//end paintScope()


	/**
	* Processes the scope options
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return void
	* @access public
	*/
	function processScope(&$asset, &$o, $prefix)
	{
		// Save the values
		if (!isset($_REQUEST[$prefix.'_options'])) return;
		$value = $_REQUEST[$prefix.'_options'];
		foreach ($value as $i => $v) {
			$value[$i] = htmlentities($v);
			if ($v == '') unset($value[$i]);
		}

		// Save the key fields
		if (!isset($_REQUEST[$prefix.'_options_keys'])) return;
		$key_value = $_REQUEST[$prefix.'_options_keys'];
		foreach ($key_value as $j => $k) {
			$key_value[$j] = htmlentities($k);
			if (empty($key_value[$j])) {
				unset($key_value[$j]);
			}//end if
		}//end foreach

		$values = Array();
		foreach ($key_value as $index => $key) {
			$values[$key] = (isset($value[$index])) ? $value[$index] : '';
		}//end foreach

		$processed_value = $values;
		if ($asset->writeAccess('attributes')) {
			$asset->setAttrValue('scope', $processed_value);
			$asset->saveAttributes();
		}//end if

		return TRUE;

	}//end processScope()


	/**
	* Print the interface for filling in a value
	*
	* @param array	$values	the values we are painting
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access private
	*/
	function _paintOptionValue($values, &$asset, &$o, $prefix)
	{
		// Check for write access
		if ($asset->writeAccess('attributes')) {
			$read_only = FALSE;
		} else {
			$read_only = TRUE;
		}

		if (!$read_only) {
			?>
			<div id="option-list-<?php echo $prefix; ?>">
				<?php
				$order = 0;
				if (count($values) > 0) {
					foreach ($values as $k => $v) {
						$k = html_entity_decode($k);
						$v = html_entity_decode($v);
						$printup = ($order < 1) ? FALSE : TRUE;
						$this->_printOptionField($k, $v, $order, $prefix, 20);
						$order++;
					}
				}
				for ($i = 0; $i < 2; $i++) {
					$k = '';
					$v = '';
					$printdown = ($i < 1) ? TRUE : FALSE;
					$this->_printOptionField($k, $v, $order, $prefix, 20);
					$order++;
				}
				?>
			</div>
			<script type="text/javascript">
			<?php
			// Yeesh... this is hackish but this essentially (attempts to)
			// correct the lack of delete-button class in front-end
			// asset builder creation
			if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			?>
				buttons = document.getElementsByTagName('BUTTON');
			for (i = 0; i < buttons.length; i++) {
				if (buttons[i].className == 'delete-button') {
					with (buttons[i].style) {
						backgroundImage = 'url(<?php echo sq_web_path('lib') ?>/web/images/icons/delete.png)';
						backgroundPosition = '50% 50%';
						backgroundRepeat = 'no-repeat';
						width = '16px';
						height = '16px';
						backgroundColor = 'transparent';
						border = '0px';
						cursor = 'pointer';
					}
				}
			}
		<?php
		}
		?>
			// Some variables
			var optionItemPrefix = '<?php echo $prefix; ?>';

			// Functions for option list attribute
			var doExpandList = new Function('doExpandOptionList(this)');
			var doDeleteRow = new Function('doDeleteOptionListRow(this); return false;');

			function doExpandOptionList(input)
			{
				// abort if we are not the last input in the lit
				var nextInput = input.nextSibling;
				while (nextInput !== null) {
					if (nextInput.tagName == 'INPUT') {
						return;
					}
					nextInput = nextInput.nextSibling;
				}

				// abort if we and the second-last input are both empty
				var lastInput = input.previousSibling;
				var last2Input = lastInput.previousSibling;
				if (input.value == '') {
					while (lastInput !== null) {
						if (lastInput.tagName == 'INPUT') {
							last2Input = lastInput.previousSibling;
							while (last2Input !== null) {
								if (last2Input.tagName == 'INPUT') {
									if (last2Input.value == '') {
										return;
									}
									break;
								}
								last2Input = last2Input.previousSibling;
							}
							break;
						}
						lastInput = lastInput.previousSibling;
					}
				}

				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var inputs = optionList.getElementsByTagName('INPUT');
	
				var brElements = lastInput.parentNode.getElementsByTagName('BR');
				input.parentNode.removeChild(brElements[brElements.length-1]);
				input.parentNode.appendChild(document.createTextNode(' '));
				input.parentNode.appendChild(document.createElement('BR'));

				// add the extra field
				var newInput = input.cloneNode(true);
				var newKeyInput = lastInput.cloneNode(true);
				var newId = (inputs.length/2);
				var newKeyId = (inputs.length/2);
				newKeyInput.onfocus = doExpandList;
				newKeyInput.value = '';
				newKeyInput.id = optionItemPrefix+'_options_keys['+newId+']';
				input.parentNode.appendChild(newKeyInput);
				input.parentNode.appendChild(document.createTextNode(' '));
				newInput.onfocus = doExpandList;
				newInput.value = '';
				newInput.id = optionItemPrefix+'_options['+newId+']';
				input.parentNode.appendChild(newInput);
				input.parentNode.appendChild(document.createTextNode(' '));
				var delButton = input.nextSibling;
				while (delButton.tagName != 'BUTTON') {
					delButton = delButton.nextSibling;
				}
				delButton = delButton.cloneNode(true);
				delButton.onclick = doDeleteRow;
				input.parentNode.appendChild(delButton);
				input.parentNode.appendChild(document.createTextNode(' '));

				input.parentNode.appendChild(document.createTextNode(' '));
				input.parentNode.appendChild(document.createElement('BR'));

			}

			function doDeleteOptionListRow(button)
			{
				var input = button.previousSibling;
				while (input.tagName != 'INPUT') {
					input = input.previousSibling;
				}

				var input2 = input.previousSibling;
				while (input2.tagName != 'INPUT') {
					input2 = input2.previousSibling;
				}

				if (input.value == '') return;

				// If the input is disabled, don't respond
				if (input.disabled) return;

				// Don't let the option list get down to a single element. Clear the field
				// instead, but leave it as two elements
				var inputs = input.parentNode.getElementsByTagName('INPUT');
				if (inputs.length <= 2) {
					input.value = '';
					return;
				}


				var brTag = button.nextSibling;
				while (brTag.tagName != 'BR') {
					brTag = brTag.nextSibling;
				}
				button.parentNode.removeChild(input2);
				button.parentNode.removeChild(input);
				button.parentNode.removeChild(brTag);
				button.parentNode.removeChild(button);
			}

			// attach the event handlers
			var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
			var	inputs = optionList.getElementsByTagName('INPUT');
			for (var j=0; j < inputs.length; j++) {
				inputs[j].onfocus = doExpandList;
			}
			var buttons = optionList.getElementsByTagName('BUTTON');
			for (var j=0; j < buttons.length; j++) {
				buttons[j].onclick = doDeleteRow;
			}

			lastOrder = <?php echo $order; ?>;

			</script>
			<?php
		} else {
			if (count($values) > 0) {
				echo '<ul>';
				foreach ($values as $value) {
					if (!empty($value)) {
						echo '<li>'.html_entity_decode($value).'</li>';
					}//end if
				}//end foreach
				echo '</ul>';
			}//end if
		}//end if

	}//end _paintOptionValue()


	/**
	* paint the option field
	*
	* @param string		$key		the key of the current field
	* @param string		$value		the value of the current field
	* @param string		$order		the order of the current field
	* @param string		$prefix		the prefix of the current form/field
	* @param int		$size		the size of the field
	*
	* @return string
	* @access private
	*/
	function _printOptionField($key, $value, $order, $prefix, $size='10')
	{
		// The order is key1=0, value1=1, key2=2, value2=3 etc. This is just the way they are ordered
		$current_button = ($order * 2) + 1;

		// Cut the key field down to half the value field
		$key_size = $size / 2;

		// Show the field
		?>
		<input type="text"  name="<?php echo $prefix; ?>_options_keys[]" value="<?php echo $key; ?>" id="<?php echo $prefix; ?>_options_keys[<?php echo $order; ?>]" size="<?=$key_size; ?>" />
		<input type="text"  name="<?php echo $prefix; ?>_options[]" value="<?php echo $value; ?>" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" size="<?=$size; ?>" />
		<button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
		<br />
		<?php

	}//end _printOptionField()


}//end class

?>
