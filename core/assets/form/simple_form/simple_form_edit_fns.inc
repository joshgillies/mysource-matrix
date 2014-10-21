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
* $Id: simple_form_edit_fns.inc,v 1.9 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Simple_Form_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Simple_Form_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Simple_Form_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


//--        QUESTION DISPLAY FUNCTIONS        --//


	/**
	* Paint the questions
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintQuestions(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($asset->id.':q1', '', TRUE);

		// TRANSLATE
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data') ?>/asset_types/simple_form/js/sort_order_manip.js"></script>
		<div style="display:none"><?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_2.png', 16, 16, '', '', 'id="'.$prefix.'_moved_up" align="absmiddle"') ?>
		<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/internal_message/priority_4.png', 16, 16, '', '', 'id="'.$prefix.'_moved_down" align="absmiddle"') ?></div>
		<table class="sq-backend-table">
		<thead>
		<tr>
			<th>
				<?php echo translate('Question'); ?>

			</th>
			<th>
				<?php echo translate('Type'); ?>

			</th>
			<?php if ($write_access) {
			?><th style="width:30px; text-align:center">
				<?php echo translate('Up'); ?>

			</th>
			<th colspan="2" style="text-align:center">
				<?php echo translate('Position'); ?>

			</th>
			<th style="width:30px; text-align:center">
				<?php echo translate('Down'); ?>

			</th>
			<th style="width:50px; text-align:center">
				<?php echo translate('Delete'); ?>

			</th><?php
			}
		?></tr>
		</thead>
		<tbody>
			<?php if (count($asset->attr('questions')) <= 0) {
				?><tr><td colspan="<?php echo $write_access ? 8 : 3 ?>" style="font-style:italic; text-align:center"><?php echo translate('This form contains no questions'); ?></td></tr><?php

			} else {
				foreach ($asset->attr('questions') as $shadowid => $asset_data) {
				$assetid = $asset->id.':q'.$shadowid;
				$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$href = $q->getBackendHref('details');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
				unset($q); ?>
				<tr id="<?php echo 	$prefix.'_order_row'.$asset_data['sort_order'] ?>">
				<td>
					<span name="<?php echo $prefix.'_order_o'.$asset_data['sort_order'] ?>" id="<?php echo $prefix.'_order_o'.$asset_data['sort_order'] ?>"><?php echo get_asset_tag_line($assetid, 'details') ?><!-- <b><a href="<?php echo $href ?>"><?php // echo $asset_data['attributes']['name']; ?></a></b> --></span><?php hidden_field($prefix.'_order[reorder]['.$asset_data['sort_order'].']', $shadowid); ?>
				</td>
				<td>
					<span name="<?php echo $prefix.'_order_t'.$asset_data['sort_order'] ?>" id="<?php echo $prefix.'_order_t'.$asset_data['sort_order'] ?>"><?php
						$q_type = str_replace('form_question_type', '', $asset_data['type_code']);
						$q_type = str_replace('_', ' ', $q_type);
						$q_type = trim(ucwords($q_type));
						echo $q_type;
					?></span>
				</td>
				<?php if ($write_access) {
					?><td style="width:30px; text-align:center">
						<?php
						if ($asset_data['sort_order'] != 0) {
							?><a href="#" onclick="moveQuestion(document.main_form, '<?php echo $prefix ?>', <?php echo $asset_data['sort_order'] ?>, <?php echo $asset_data['sort_order'] - 1 ?>); return false;">
								<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/up_arrow.png', 16, 16, "Move Up"); ?>
							</a><?php
						}
						?>
					</td>
					<td style="width:30px; text-align:right">
						<span id="<?php echo $prefix.'_order_so'.$asset_data['sort_order'] ?>"><?php echo $asset_data['sort_order'] + 1; ?></span>
					</td>
					<td style="width:30px; text-align:left">
						<span id="<?php echo $prefix.'_order_soa'.$asset_data['sort_order'] ?>"></span><span style="visibility:hidden; font-weight: bold" name="<?php echo $prefix.'_order_soc'.$asset_data['sort_order'] ?>" id="<?php echo $prefix.'_order_soc'.$asset_data['sort_order'] ?>"><?php echo $asset_data['sort_order'] + 1; ?></span>
					</td>
					<td style="width:30px; text-align:center">
						<?php
						if ($asset_data['sort_order'] != count($asset->attr('questions')) -1) {
							?><a href="#" onclick="moveQuestion(document.main_form, '<?php echo $prefix ?>', <?php echo $asset_data['sort_order'] ?>, <?php echo $asset_data['sort_order'] + 1 ?>); return false;">
								<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/down_arrow.png', 16, 16, "Move Down"); ?>
							</a><?php
						}
						?>
					</td>
					<td style="width:50px; text-align:center">
						<?php check_box($prefix.'_order[delete]['.$asset_data['sort_order'].']'); ?>
					</td>
				<?php }
				?></tr>
			<?php }//end foreach
			}//end else
		?></tbody>
		</table>
		<?php

		return $write_access;

	}//end paintQuestions()


	/**
	* Process the questions
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_order']['reorder'])) {
			return FALSE;
		}

		$old_questions = $asset->attr('questions');
		$questions = Array();

		$old_sort_order = 0;
		$new_sort_order = 0;
		foreach ($_POST[$prefix.'_order']['reorder'] as $order_shadowid) {
			if (empty($_POST[$prefix.'_order']['delete'][$old_sort_order])) {
				$questions[$order_shadowid] =& $old_questions[$order_shadowid];
				$questions[$order_shadowid]['sort_order'] = $new_sort_order;
				$new_sort_order++;
			}
			$old_sort_order++;
		}

		$asset->setAttrValue('questions', $questions);
		return TRUE;

	}//end processQuestions()


	/**
	* Paint the dropdowns to add questions to this section
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			?><i><?php echo translate('Lock this asset to add questions'); ?></i><?php

			return FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$question_types = $am->getTypeDescendants('form_question');
		$questions = Array('' => '-- '.translate('Choose Type').' --');


		foreach ($question_types as $question) {
			$q = str_replace('form_question_type', '', $question);
			$q = str_replace('_', ' ', $q);
			$q = trim(ucwords($q));
			$questions[$question] = $q;
		}

		$prefix = str_replace(':','_',$prefix); // handle shadow assets
		?>
		<div id="question-list-<?php echo $prefix; ?>">
			<?php
			for ($i =0; $i < 2; $i++) {
				?>
				<div><input type="text" name="<?php echo $prefix; ?>_add[names][]" /><?php echo combo_box($prefix.'_add[types][]', $questions, FALSE, ''); ?><button type="button" tabindex="99999" class="delete-button">&nbsp;</button></div>
				<?php
			}
			?>
		</div>
		<script type="text/javascript">
			// attach the event handlers
			var optionList = document.getElementById('question-list-<?php echo $prefix; ?>');
			var	divs = optionList.getElementsByTagName('DIV');
			for (var i=0; i < divs.length; i++) {
				var	thisDiv = divs[i];

				var	inputs = thisDiv.getElementsByTagName('INPUT');
				for (var j=0; j < inputs.length; j++) {
					inputs[j].onfocus = expandListFormFn;
				}
				var	selects = thisDiv.getElementsByTagName('SELECT');
				for (var j=0; j < selects.length; j++) {
					selects[j].onfocus = expandListFormFn;
				}
				var buttons = thisDiv.getElementsByTagName('BUTTON');
				for (var j=0; j < buttons.length; j++) {
					buttons[j].onclick = deleteRowFormFn;
				}
			}
		</script>
		<?php
		return TRUE;

	}//end paintAddQuestions()


	/**
	* Process the newly added questions
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddQuestions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$names = $_POST[$prefix.'_add']['names'];
		$types = $_POST[$prefix.'_add']['types'];

		// this can eventually be expanded to handle named questions
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		foreach ($types as $i => $type) {
			if (empty($type)) continue;

			$am->includeAsset($type);
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// create some questions
			if (!$asset->createQuestion($type, $names[$i])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}//end if

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end foreach

		return TRUE;

	}//end processAddQuestions()


	/**
	* Process use format bodycopy dropdown
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUseBodycopy(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', 'format');
		$enabled = !empty($bodycopy_link);

		if ($write_access) {
			$options = Array(
						'1'	=> translate('Yes'),

						'0'	=> translate('No'),

					   );

			combo_box($prefix.'_use_bodycopy', $options, FALSE, Array($enabled));

		} else {
			echo $enabled ? translate('On') : translate('Off');


		}

		return $write_access;

	}//end paintUseBodycopy()


	/**
	* Process use format bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUseBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		$enabled = $_POST[$prefix.'_use_bodycopy'];

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$asset->setUseFormatBodycopy($enabled)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processUseBodycopy()


}//end class


?>
