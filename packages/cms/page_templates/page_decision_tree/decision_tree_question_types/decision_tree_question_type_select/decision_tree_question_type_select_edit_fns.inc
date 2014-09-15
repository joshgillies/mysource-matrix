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
* $Id: decision_tree_question_type_select_edit_fns.inc,v 1.3 2013/06/04 08:54:31 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_decision_tree/decision_tree_question/decision_tree_question_edit_fns.inc';


/**
* Decision_Tree_Question_Type_Select_Edit_Fns
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Type_Select_Edit_Fns extends Decision_Tree_Question_Edit_Fns
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
	* Print the interface for filling in a value
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function paintOptions(&$asset, &$o, $prefix)
	{
		// Check for write access
		if ($asset->writeAccess('attributes')) {
			$read_only = FALSE;
		} else {
			$read_only = TRUE;
		}

		$prefix = str_replace(':','_',$prefix); // handle shadow assets
		$value = $asset->attr('options');

		if (!$read_only) {
			?>
				<div id="option-list-<?php echo $prefix; ?>">
				<?php
				$order = 0;
				if (count($value) > 0) {
					foreach ($value as $k => $v) {
						$k = html_entity_decode($k, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
						$v = html_entity_decode($v, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
						$printup = ($order < 1) ? FALSE : TRUE;
						$this->_printOptionField($k, $v, $order, $prefix, 20, $printup);
						$order++;
					}
				}
				for ($i = 0; $i < 2; $i++) {
					$k = '';
					$v = '';
					$printdown = ($i < 1) ? TRUE : FALSE;
					$this->_printOptionField($k, $v, $order, $prefix, 20, TRUE, $printdown);
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
			var doMoveDn = new Function('sListMoveDown(this); return false;');
			var doMoveUp = new Function('sListMoveUp(this); return false;');

			function sListMoveDown(obj) {
				var currentOrder = 0;

				var list = document.getElementById('option-list-<?php echo $prefix; ?>');
				var inputs = list.getElementsByTagName('INPUT');
				for (var i=0 ; i < inputs.length; i++) {
					if (obj.id == inputs[i].id) {
						currentOrder = i;
						break;
					}
				}

				if (currentOrder == inputs.length) return;

				// If the input is disabled, don't respond
				if (inputs[currentOrder].disabled) return;

				// Do the move
				if (currentOrder > 2) {
					sListMoveMe(currentOrder, currentOrder-2);
				}
			}

			function sListMoveUp(obj) {
				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var currentOrder = 0;

				var inputs = optionList.getElementsByTagName('INPUT');
				for (var i=0 ; i < inputs.length; i++) {
					if (obj.id == inputs[i].id) {
						currentOrder = i;
						break;
					}
				}

				if (currentOrder == inputs.length) return;

				// If the input is disabled, don't respond
				if (inputs[currentOrder].disabled) return;

				// Do the move
				if (currentOrder > 0) {
					sListMoveMe(currentOrder-2, currentOrder);
				}
			}

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

				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var inputs = optionList.getElementsByTagName('INPUT');

				// add move down button to the previous input
				var moveDownButton = lastInput.previousSibling;
				while (moveDownButton != null) {
					moveDownButton = moveDownButton.previousSibling;
					if (moveDownButton.tagName == 'A' && moveDownButton.name=="movedown") {
						break;
					}
				}
				var newMoveDownButton = moveDownButton.cloneNode(true);
				newMoveDownButton.id = optionItemPrefix+'_options['+(inputs.length/2)+']';
				newMoveDownButton.onclick = doMoveDn;


				//If safari, we will remove the script for printing move up/down icon, it's causing document.write to overwrite the page in safari
				var buttonScript =  newMoveDownButton.getElementsByTagName("script")[0];
				var browserAgent = navigator.userAgent.toLowerCase();
				if ((browserAgent.indexOf("safari") != -1) && buttonScript != null) {
					newMoveDownButton.removeChild(buttonScript);
				}



				var brElements = lastInput.parentNode.getElementsByTagName('BR');
				input.parentNode.removeChild(brElements[brElements.length-1]);
				input.parentNode.appendChild(document.createTextNode(' '));
				input.parentNode.appendChild(newMoveDownButton);
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

				// add the move up button to the new input
				var moveUpButton = input.nextSibling;
				while (moveUpButton != null) {
					if (moveUpButton.tagName == 'A' && moveUpButton.name == 'moveup') {
						break;
					}
					moveUpButton = moveUpButton.nextSibling;
				}
				moveUpButton = moveUpButton.cloneNode(true);
				moveUpButton.id = optionItemPrefix+'_options['+((inputs.length/2)-1)+']';
				moveUpButton.onclick = doMoveUp;

				//If safari, we will remove the script for printing move up/down icon, it's causing document.write to overwrite the page in safari
				var buttonScript =  moveUpButton.getElementsByTagName("script")[0];
				var browserAgent = navigator.userAgent.toLowerCase();
				if ((browserAgent.indexOf("safari") != -1) && buttonScript != null) {
						moveUpButton.removeChild(buttonScript);
				}

				input.parentNode.appendChild(moveUpButton);
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

				var moveUpBut = button.nextSibling;
				while (moveUpBut != null) {
					if (moveUpBut.tagName == 'A' && moveUpBut.name == 'moveup') {
						break;
					}
					moveUpBut = moveUpBut.nextSibling;
				}
				button.parentNode.removeChild(moveUpBut);

				var moveDownBut = button.nextSibling;
				while (moveDownBut != null) {
					if (moveDownBut.tagName == 'A' && moveDownBut.name == 'movedown') {
						break;
					}
					moveDownBut = moveDownBut.nextSibling;
				}
				button.parentNode.removeChild(moveDownBut);


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

			function sListMoveMe(first, second) {
				var l = document.getElementById('option-list-<?php echo $prefix; ?>');
				var f = l.getElementsByTagName('INPUT');
				var tempkey = f[first-1].value;

				f[first-1].value = f[second-1].value;
				f[second-1].value = tempkey;

				var temp = f[first].value;
				f[first].value = f[second].value;
				f[second].value = temp;
			}

		</script>
		<?php
		} else {
			if (count($value) > 0) {
				echo '<ul><li>'.html_entity_decode(implode('</li><li>', $value), ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</li></ul>';
			}
		}

	}//end paintOptions()


	/**
	* Process the interface for filling in a value
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function processOptions(&$asset, &$o, $prefix)
	{
		$prefix = str_replace(':','_',$prefix);	// handle shadow assets

		// Save the values
		if (!isset($_REQUEST[$prefix.'_options'])) return;
		$value = $_REQUEST[$prefix.'_options'];
		foreach ($value as $i => $v) {
			$value[$i] = htmlentities($v, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			if ($v == '') unset($value[$i]);
		}

		// Save the key fields
		if (!isset($_REQUEST[$prefix.'_options_keys'])) return;
		$key_value = $_REQUEST[$prefix.'_options_keys'];

		// Restrict the allowed characters in the "key"
		$key_value = preg_replace('/[^A-Za-z0-9\-]/', '_', $key_value);

		foreach ($key_value as $j => $k) {
			$key_value[$j] = htmlentities($k, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);
			if (($k == '') && (isset($value[$j]) && !($value[$j] == ''))) {
				$key_value[$j] = $value[$j];
			} else if (($k == '') && (!isset($value[$j]) || ($value[$j] == ''))) {
				unset($key_value[$j]);
			}
		}

		$key_value_count = count($key_value);
		$value_count = count($value);
		if ($key_value_count != $value_count) return;
		if ($key_value_count > 0) {
			$values = array_combine($key_value, $value);
		} else {
			$values = Array();
		}

		$processed_value = $values;
		if ($asset->writeAccess('attributes')) {
			$asset->setAttrValue('options', $processed_value);
			$asset->saveAttributes();
		}

	}//end processOptions()


	/**
	* paint the down arrow button
	*
	* @param string	$order	Order of the element
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function _printMoveDownButton($order, $prefix)
	{
		// We don't need to check order here because the check are in place for the buttons
		$orderdn = $order + 2;
		?>
			<a href="#" style="cursor:pointer;" onclick="sListMoveMe(<?php echo $order; ?>, <?php echo ($orderdn); ?>);return false;" tabindex="99999" name="movedown" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
		<?php

	}//end _printMoveDownButton()


	/**
	* paint the up arrow button
	*
	* @param string	$order	Order of the element
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function _printMoveUpButton($order, $prefix)
	{
		// We don't need to check order here because the check are in place for the buttons
		$orderup = $order - 2;
		?>
			<a href="#" style="cursor:pointer;" onclick="sListMoveMe(<?php echo $order; ?>, <?php echo ($orderup); ?>);return false;" tabindex="99999" name="moveup" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
		<?php

	}//end _printMoveUpButton()


	/**
	* paint the option field
	*
	* @param string		$key		the key of the current field
	* @param string		$value		the value of the current field
	* @param string		$order		the order of the current field
	* @param string		$prefix		the prefix of the current form/field
	* @param int		$size		the size of the field
	* @param boolean	$showup		Show the up button
	* @param boolean	$showdown	Show the down button
	*
	* @return string
	* @access private
	*/
	function _printOptionField($key, $value, $order, $prefix, $size='10', $showup=TRUE, $showdown=TRUE)
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
		<?php
			if ($showup) {
				$this->_printMoveUpButton($current_button, $prefix);
			}
			if ($showdown) {
				$this->_printMoveDownButton($current_button, $prefix);
			}
		?><br />
		<?php

	}//end _printOptionField()


	/**
	* Paint a table for the destination data for each key of the 'options' attribute.
	* Destination data includes the next question, result bodycopy and points value.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDestinations(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$options = $asset->getOptions();
		$q_dest = $asset->attr('question_destinations');

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('answer_key'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('next_question'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('result_bodycopy'); ?>
				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('points_value'); ?>
				</td>
			</tr>
		<?php
		foreach ($options as $key => $value){
			if ($key === $asset->attr('empty_key')) continue;
			?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					echo $key;
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$nxt_q = (!empty($q_dest[$key]['next_question'])) ? $q_dest[$key]['next_question'] : '';
				if ($write_access){
					$type_codes = Array (
									  'decision_tree_question_type_select' 	=> 'I',
									  'decision_tree_question_type_numeric' => 'I',
									);
					asset_finder($prefix.'_next_question_'.$key, $nxt_q, $type_codes);
				} else {
					if ($nxt_q) echo get_asset_tag_line($nxt_q, 'details');
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$res_body = (!empty($q_dest[$key]['result_bodycopy'])) ? $q_dest[$key]['result_bodycopy'] : '';
				if ($write_access){
					$type_codes = Array (
									  'bodycopy' 	=> 'I',
									);
					asset_finder($prefix.'_bodycopy_'.$key, $res_body, $type_codes);
				} else {
					if ($res_body) echo get_asset_tag_line($res_body, 'details');
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$points = (!empty($q_dest[$key]['points_value'])) ? $q_dest[$key]['points_value'] : 0;
				if ($write_access){
					text_box($prefix.'_points_value_'.$key, $points);
				} else {
					echo $points;
				}
				?>
				</td>
			</tr>
		<?php }
		?>
		</table>
		<?php
		return TRUE;
	}//end paintDestinations()


	/**
	* Processes a table for the destination data for each key of the 'options' attribute.
	* Destination data includes the next question, result bodycopy and points value.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDestinations(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$options = $asset->getOptions();
		$q_dest = Array();

		foreach ($options as $key => $value){
			if ($key === $asset->attr('empty_key')) continue;
			$nxt_q = (isset($_POST[$prefix.'_next_question_'.$key])) ? $_POST[$prefix.'_next_question_'.$key]['assetid'] : '';
			if (!empty($nxt_q)){
				//check if its a child else create a link under it
				$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($nxt_q, 'decision_tree_question', FALSE, NULL, NULL, TRUE, 1, 1);
				if (!array_key_exists($asset->id, $parents)){
					$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($nxt_q);
					if(!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $minor, SQ_LINK_TYPE_2)) $nxt_q = '';
				}
			}
			$q_dest[$key]['next_question'] = $nxt_q;

			$res_body = (isset($_POST[$prefix.'_bodycopy_'.$key])) ? $_POST[$prefix.'_bodycopy_'.$key]['assetid'] : '';
			$q_dest[$key]['result_bodycopy'] = $res_body;

			$points = (!empty($_POST[$prefix.'_points_value_'.$key])) ? $_POST[$prefix.'_points_value_'.$key] : 0;

			if (!is_numeric($points)) {
				trigger_localised_error('CMS0114', E_USER_NOTICE);
				$points = 0;
			}
			$q_dest[$key]['points_value'] = $points;

		}

		$asset->setAttrValue('question_destinations', $q_dest);
		$asset->saveAttributes();

		return TRUE;
	}//end processDestinations()



}//end class
?>
