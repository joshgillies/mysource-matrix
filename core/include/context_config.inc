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
* $Id: context_config.inc,v 1.11 2012/08/30 01:09:19 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Context_Config
*
* Purpose
*    System configuration screen for the managing of contexts, for
*    the purposes of multilingual/multiple asset content.
*
*    This config screen is different from the usual, since it does
*    NOT write to a data/private/conf file. It instead writes its
*    details to the sq_ctx_def table. This is because information
*    tied to context IDs (eg. attribute and metadata values) are
*    open to rollback, so the information behind these context IDs
*    should be open to it too.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix
*/
class Context_Config extends Config
{

	/**
	* The array of default messaging config settings
	* @var array
	*/
	var $config_vars = Array(
						// Proxy Options
						'SQ_CONTEXT_DEFINITIONS'	=> Array('editable' => 1, 'default' => 'a:0:{}'),
					   );

	/**
	* Prefix for form elements, and screen-specific translation strings
	*
	* Note that this is not necessary for common words being translated,
	* but for headings and other elements of the interface, this prefix
	* should be used to identify it as part of the Context Config screen.
	*
	* @var string
	*/
	var $prefix = 'config_context_';


	/**
	* Array of newly-set variables
	*
	* @var array
	*/
	var $new_vars = NULL;


	/**
	* Constructor
	*
	* @return void
	*/
	function __construct()
	{
		$this->config_vars['SQ_CONTEXT_DEFINITIONS']['default'] = serialize(Array('0' => Array('name' => 'Default Context'))); 

		parent::__construct();
		$this->config_file = NULL;

	}//end constructor


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	Backend_Outputter, reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		$contexts = $this->load();
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class_lower($this);

		// Post variables
		if ($write_access === TRUE) {
			$current_context = array_get_index($_POST, $this->prefix.'selected');
			if (!is_numeric($current_context)) {
				// Don't let bad data get the better of this screen
				$current_context = NULL;
			} else {
				$current_context = (int)$current_context;
			}

			if ($current_context == NULL) {
				// We can only be editing a context if we haven't selected one
				// in the same page request.
				$editing_context = array_get_index($_POST, $this->prefix.'editing');
				if (!is_numeric($editing_context)) {
					$editing_context = NULL;
				} else {
					$editing_context = (int)$editing_context;
				}
			}
		} else {
			// Can't have a "current context" while the screen is
			// not locked!
			$current_context = NULL;
		}

		// Summary list of contexts
		$o->openSection(translate($this->prefix.'list_of_contexts'));
			$o->openRaw('');
				$this->paintSummary($o, $contexts, $write_access, $current_context);
			$o->closeRaw();
			$o->sectionNote(translate($this->prefix.'note'));

		$o->closeSection();

		// Edit currently selected context (as selected through the post var)
		if (($write_access === TRUE) && (is_null($current_context) === FALSE)) {
			if (isset($contexts[$current_context]) === TRUE) {
				$o->openSection(translate($this->prefix.'edit_selected_context'));
					$this->paintSelectedContext($o, $contexts, $current_context);
				$o->closeSection();
			}
		}

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Paints the summary of contexts, and interface to allow select one for editing
	*
	* @param Backend_Outputter	$o					The backend outputter being used
	* @param array				$contexts			The list of currently available contexts
	*                                      			  (ie. SQ_CONTEXT_DEFINITIONS)
	* @param boolean			$write_access		TRUE if write access available
	* @param int				$current_context	Currently selected context. Used for
	*												highlighting the selection being edited.
	*
	* @return void
	*/
	public function paintSummary(Backend_Outputter $o, Array $contexts, $write_access, $current_context=NULL)
	{
		?>
<?php hidden_field($this->prefix.'selected', ''); ?>
<?php hidden_field($this->prefix.'deleted', ''); ?>
<table class="sq-backend-table" style="margin-bottom: 1em">
	<colgroup>
		<col width="60" />
		<col width="20%" />
		<col />
		<col width="60" />
		<col width="120" />
		<col width="60" />
	</colgroup>
	<tbody>
<?php
		// Keep a running tally of number of alternate
		// contexts, so we can offer a default name to it
		$alt_context_num = 0;
		
		// Make sure default context is up top, sort the
		// rest by sort order
		$default_context = Array(0 => $contexts[0]);
		unset($contexts[0]);
		uasort($contexts, create_function('$a,$b', 'return $a["sort_order"] - $b["sort_order"];'));
		$contexts = $default_context + $contexts;

		foreach ($contexts as $contextid => $context) {
			$context_name = array_get_index($context, 'name', '');
			$context_errors = $this->_getContextErrors($context);
			$condition_err_count = 0;
			
			if ($alt_context_num === 0) {

				?>
	<tr class="sq-table-row">
		<th class="sq-backend-table-header-header" colspan="6">
			<?php
				echo translate($this->prefix.'default_name'); ?>
		</th>
	</tr>
	<tr>
		<th><?php echo translate('edit_question'); ?></th>
		<th><?php echo translate($this->prefix.'context_name'); ?></th>
		<th colspan="4"></th>
	</tr>
	<?php
			}

			if ($current_context === (int)$contextid) {
				$row_class = 'sq-table-row alt';
			} else {
				$row_class = 'sq-table-row';
			}
			?>
	<tr class="<?php echo $row_class; ?>">
		<td class="sq-backend-table-cell" align="center"><?php
			if (($write_access === TRUE) && ((int)$contextid !== $current_context)) { ?>
			<p><a href="#" onclick="document.getElementById('<?php echo $this->prefix; ?>selected').value = <?php echo $contextid ?>; document.main_form.submit(); return false;"><?php echo trim(sq_get_icon(sq_web_path('lib').'/web/images/icons/edit_mode.png', 15, 15, translate($this->prefix.'edit_alt_text', $context_name), NULL, ' border="0"')); ?></a></p><?php
			}
		?>
		</td>
		<td class="sq-backend-table-cell">
			<strong><?php echo $context_name; ?></strong><?php
			if ($contextid !== 0) {
				hidden_field($this->prefix.'reorder[]', $contextid, ' id="'.$this->prefix.'reorder_'.$contextid.'"');
			}
		?>
		</td>
		<td class="sq-backend-table-cell"><?php
			if ($contextid === 0) {
				echo translate($this->prefix.'default_explanation');
				$valid = TRUE;
			} else {
				if ($context['at_least'] === NULL) {
					echo translate($this->prefix.'table_all_description', count($context['conditions']));
				} else {
					echo translate($this->prefix.'table_at_least_description', $context['at_least'], count($context['conditions']));
				}

				?><ul>
				<?php
				$condition_text = $this->_getSummaryConditionList($context['conditions']);
				foreach ($condition_text as $condition_id => $condition_text_item) {
					?><li><?php echo $condition_text_item;
					$condition_errors = array_get_index($context_errors, $condition_id, Array());
					if (count($condition_errors) > 0) {
						$condition_err_count++;
						?><ul>
						<?php
							foreach ($condition_errors as $condition_error) {
								?><li class="sq-backend-warning"><?php echo $condition_error; ?></li><?php
							}
						?></ul><?php
					}
					?></li>
					<?php
				}

				$condition_level_errors = array_get_index($context_errors, 'condition', Array());
				if (count($condition_level_errors) > 0) {
					foreach ($context_level_errors as $context_level_error) {
						?><li class="sq-backend-warning"><?php echo $condition_level_error; ?></li><?php
					}
				}


				?></ul>
				<?php
				$context_level_errors = array_get_index($context_errors, 'context', Array());
				if (count($context_level_errors) > 0) {
					foreach ($context_level_errors as $context_level_error) {
						?><p class="sq-backend-warning"><?php echo $context_level_error; ?></p><?php
					}
				}

				if (count($context['conditions']) === 0) {
					$valid = FALSE;
				} else if ($context['at_least'] === NULL) {
					$valid = (count($context_errors) === 0) ? TRUE : FALSE;
				} else {
					$valid = ((count($context['conditions']) - $condition_err_count) >= $context['at_least']) ? TRUE : FALSE;
				}
				
				if ($condition_err_count > 0) {
					if ($valid === TRUE) {
						?><p><?php
							echo translate($this->prefix.'invalid_cond_but_still_valid');
						?></p><?php
					} else {
						?><p class="sq-backend-warning"><?php
							echo translate($this->prefix.'not_enough_valid_cond_to_be_valid');
						?></p><?php
					}
				}
			}
		?>
		</td>
		<td class="sq-backend-table-cell" align="center"><?php
			// valid icon
			// Context is valid IF:
			//  ALL conditions are required, and no context errors,
			//  AT LEAST <x> conditions are required, and
			//    there are at least that many valid conditions.
			if ($valid) {
				echo trim(sq_get_icon(sq_web_path('lib').'/web/images/tick.gif', 15, 15, translate($this->prefix.'valid_alt_text', $context_name), NULL, ' border="0"'));
			} else {
				echo trim(sq_get_icon(sq_web_path('lib').'/web/images/cross.gif', 15, 15, translate($this->prefix.'not_valid_alt_text', $context_name), NULL, ' border="0"'));
			}
		?>
		</td>
		<td class="sq-backend-table-cell" align="center"><?php
			// reorder link
			if ($write_access === TRUE) {
				if ($alt_context_num > 1) {				
					?><a href="#" onclick="return false;" class="reorder-up"><?php echo trim(sq_get_icon(sq_web_path('lib').'/web/images/icons/up_arrow.png', 15, 15, translate($this->prefix.'reorder_up_alt_text', $context_name), NULL, ' border="0"')); ?></a><?php				
				} else {
					//no need to print anything			
				}
				?> <?php
				if (($contextid !== 0) && ($alt_context_num < count($contexts) - 1)) {
					?><a href="#" onclick="return false;" class="reorder-down"><?php echo trim(sq_get_icon(sq_web_path('lib').'/web/images/icons/down_arrow.png', 15, 15, translate($this->prefix.'reorder_down_alt_text', $context_name), NULL, ' border="0"')); ?></a><?php
				} else {
					//no need to print anything					
				}
			}
		?>
		</td>
		<td class="sq-backend-table-cell" align="center"><?php
			// delete link
			if (($write_access === TRUE) && ($contextid !== 0) && (($contextid !== $current_context))) {
				?><p><a href="#" onclick="if (confirm('<?php echo str_replace("\n", '\\n', translate($this->prefix.'delete_warning', $context_name)); ?>') == true) { document.getElementById('<?php echo $this->prefix; ?>deleted').value = <?php echo $contextid ?>; document.main_form.submit(); } return false;"><?php echo trim(sq_get_icon(sq_web_path('lib').'/web/images/icons/delete.png', 15, 15, translate($this->prefix.'delete_alt_text', $context_name), NULL, ' border="0"')); ?></a></p><?php
				
			}
		?>
		</td>
	</tr><?php
			if ($alt_context_num === 0) {
				// Print the alternate headers row here, because it should always be
				// printed even if no alternates exist (see "add new alternate"
				// row below)
				?>
	<tr class="sq-table-row">
		<th class="sq-backend-table-header-header" colspan="6">
			<?php
				echo translate($this->prefix.'alternate_contexts'); ?>
		</th>
	</tr><?php
				if (count($contexts) > 1) { 
					// print second header row for alternate contexts 
		?>
	<tr class="sq-table-row">
		<th><?php echo translate('edit_question'); ?></th>
		<th><?php echo translate($this->prefix.'context_name'); ?></th>
		<th><?php echo translate($this->prefix.'context_details'); ?></th>
		<th style="text-align: center"><?php echo translate('valid_question'); ?></th>
		<th style="text-align: center"><?php echo translate('reorder_question'); ?></th>
		<th style="text-align: center"><?php echo translate('delete_question'); ?></th>
	</tr><?php
				} else {
					// No alternate contexts - don't print header; instead print message
					// and a "lock screen to add" message if screen unlocked
		?>
	<tr class="sq-table-row">
		<td class="sq-backend-table-cell"></td>
		<td class="sq-backend-table-cell" colspan="5"><?php echo translate($this->prefix.'no_alternate_contexts'); ?></td>
	</tr><?php
				}
			}

			$alt_context_num++;
		}

		if ($write_access) {
			// Add a new alternate context
			?>
		<tr class="sq-table-row">
			<th class="sq-backend-table-header-header" colspan="6">
				<?php
					echo translate($this->prefix.'new_alternate_context'); ?>
			</th>
		</tr>
		<tr>
			<td></td>
			<td colspan="2"><?php
			if ($write_access) {
				ob_start();
					check_box($this->prefix.'new_alternate_context', '1', FALSE, 'document.getElementById("'.$this->prefix.'new_alternate_context_name").disabled = !this.checked; if (this.checked == true) { document.getElementById("'.$this->prefix.'new_alternate_context_name").focus() };');
				$check_box = ob_get_clean();
				ob_start();
					text_box($this->prefix.'new_alternate_context_name', '', 20, 0, FALSE, ' disabled="disabled"');
				$text_box = ob_get_clean();
				echo translate($this->prefix.'new_alternate_context_line', $check_box, $text_box);
			} else {
				?><br/><?php
				echo translate($this->prefix.'lock_to_add_alternate');				
			}
			?>
			</td>
			<td colspan="3"></td>
		</tr><?php			
		}
		?>
	</tbody>
</table><?php

		// paint a hidden field for re-ordering
		echo $this->_getReorderJS();
		$o->addOnLoad('ContextConfig.setup();');

	}//end paintSummmary()


	/**
	* Return fragment code required for reordering the conditions
	*
	* This script creates a top-level "ContextConfig" object, containing
	* the required code. To set up the reordering in the backend, use:
	* $o->addOnLoad('ContextConfig.setup();');
	* Where $o is a Backend_Outputter object.
	*
	* @return string
	*/
	protected function _getReorderJS()
	{
		ob_start();
		?>
		<script type="text/javascript">
		/* <![CDATA[ */
			ContextConfig = {
				moveUp:		function(link) {
								row = link.parentNode.parentNode;
								switch_row = row.previousSibling;

								while ((switch_row.nodeType != 1) || (switch_row.nodeName.toLowerCase() != 'tr')) {
									switch_row = switch_row.previousSibling;
								}

								row_tds = row.getElementsByTagName('td');
								switch_row_tds = switch_row.getElementsByTagName('td');
								var is_ie8 = navigator.userAgent.toLowerCase().indexOf("msie 8") != -1;

								for (var i = 0; i < row_tds.length; i++) {
									var swap_tds = false;
									if (is_ie8) {
										// IE 8 is special, innerhtml show the attribute value without the quote
										if (row_tds[i].innerHTML.indexOf('class=reorder-') === -1) swap_tds = true;
									} else {
										if (row_tds[i].innerHTML.indexOf('class="reorder-') === -1) swap_tds = true;
									}

									// Only reorder when it's not the "reorder" column 
									if (swap_tds) {
										var temp = switch_row_tds[i].innerHTML;
										switch_row_tds[i].innerHTML = row_tds[i].innerHTML;
										row_tds[i].innerHTML = temp;
									}
								}

								var temp_style = row.className;
								row.className = switch_row.className;
								switch_row.className = temp_style;
							},

				moveDown:	function(link) {
								row = link.parentNode.parentNode;
								switch_row = row.nextSibling;

								while ((switch_row.nodeType != 1) || (switch_row.nodeName.toLowerCase() != 'tr')) {
									switch_row = switch_row.nextSibling;
								}

								row_tds = row.getElementsByTagName('td');
								switch_row_tds = switch_row.getElementsByTagName('td');
								var is_ie8 = navigator.userAgent.toLowerCase().indexOf("msie 8") != -1;

								for (var i = 0; i < row_tds.length; i++) {
									var swap_tds = false;
									if (is_ie8) {
										// IE 8 is special, innerhtml show the attribute value without the quote
										if (row_tds[i].innerHTML.indexOf('class=reorder-') === -1) swap_tds = true;
									} else {
										if (row_tds[i].innerHTML.indexOf('class="reorder-') === -1) swap_tds = true;
									}

									// Only reorder when it's not the "reorder" column 
									if (swap_tds) {
										var temp = switch_row_tds[i].innerHTML;
										switch_row_tds[i].innerHTML = row_tds[i].innerHTML;
										row_tds[i].innerHTML = temp;
									}
								}

								var temp_style = row.className;
								row.className = switch_row.className;
								switch_row.className = temp_style;
							},
			
				setup:		function() {
								var self = this;
								var a_tags = document.getElementsByTagName('a');
								for (var i = 0; i < a_tags.length; i++) {
									a_tag = a_tags[i];
									if (a_tag.className == 'reorder-up') {
										a_tag.onclick = function(event) {
											self.moveUp(this);
											return false;
						   				}
									} else if (a_tag.className == 'reorder-down') {
										a_tag.onclick = function(event) {
											self.moveDown(this);
											return false;
					   					}
									}
								}
							}
			};

		/* ]]> */
		</script>
		<?php
		$js = ob_get_clean();
		return $js;

	}//end _getReorderJS()


	/**
	* Generates a human-readable summary from the condition list
	* of a contexts
	*
	* Returns an array of readable condition summaries. This can
	* then be styled as desired in the summary table.
	*
	* @param array	$conditions	The list of conditions
	*
	* @return array
	*/
	protected function _getSummaryConditionList($conditions)
	{
		$condition_text = Array();

		foreach ($conditions as $condition_id => $condition_details) {
			$options = $condition_details['options'];

			switch ($condition_details['type']) {
				case 'accept-language':
					$locales = $this->_getAcceptLanguageOptions();
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_accept-language', $locales[$options['language']], number_format($options['min_q'], 1));
				break;

				case 'header':
					$translate_suffix = ((int)$options['match_logic'] === 1 ? '' : '!').$options['match_type'];
					$case_sensitive_text = ((int)($options['case_sensitive']) === 1 ? '' : 'not_').'case_sensitive';
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_header_'.$translate_suffix, $options['header_name'], $options['match_value'], translate($case_sensitive_text));
				break;

				case 'get':
				case 'post':
					$translate_suffix = ((int)$options['match_logic'] === 1 ? '' : '!').$options['match_type'];
					$case_sensitive_text = ((int)($options['case_sensitive']) === 1 ? '' : 'not_').'case_sensitive';
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_'.$condition_details['type'].'_'.$translate_suffix, $options['var_name'], $options['match_value'], translate($case_sensitive_text));
				break;

				case 'url':
					$translate_suffix = ((int)$options['match_logic'] === 1 ? '' : '!').$options['match_type'];
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_url_'.$translate_suffix, $options['match_value']);
				break;

				case 'port':
					$translate_suffix = ($options['match_type'] === 'is' ? '' : '_!');
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_port'.$translate_suffix, $options['match_value']);
				break;

				case 'secure':
					$translate_suffix = ($options['match_value'] === 1 ? '' : '_!');
					$condition_text[$condition_id] = translate($this->prefix.'summary_cond_secure'.$translate_suffix);
				break;
			}
		}

		return $condition_text;

	}//end _getSummaryConditionList()


	/**
	* Returns a list of errors with this context
	*
	* If the context is valid, no errors will be returned
	* (the return will be empty array). If the return is
	* not empty, assume the context is NOT valid.
	*
	* Context errors will be keyed in two ways:
	*    'context'		Whole-of-context-level errors.
	*					(for instance, requiring "at least 3"
	*					with only two conditions)
	*
	*    'condition'	Condition-level errors caused by a
	*					combination of conditions. For instance,
	*					conditions that negate each other.
	*
	*    integer key	Condition-level errors affecting a
	*					single condition. Key is
	*					the condition ID.
	*
	* The splitting of context errors by condition allows the
	* paint functions to paint the errors next to/underneath
	* the condition to which it applies.
	*
	* @param array	$context	Details of a context
	*
	* @return array
	*/
	protected function _getContextErrors($context)
	{
		$context_errors = Array();

		if (count($context['conditions']) < 1) {
			$context_errors['context'][] = translate($this->prefix.'context_has_no_conditions');
		} else if ((is_numeric($context['at_least']) === TRUE) && ($context['at_least'] > count($context['conditions']))) {
			$context_errors['context'][] = translate($this->prefix.'context_at_least_too_many', $context['at_least'], count($context['conditions']));
		} else {
			foreach ($context['conditions'] as $condition_id => $condition) {
				$condition_type    = $condition['type'];
				$condition_options = $condition['options'];

				switch ($condition_type) {
					case 'url':
						if ($condition_options['match_value'] === '') {
							$context_errors[$condition_id][] = translate($this->prefix.'error_cond_url_no_match_value');
						}
					break;

					case 'header':
						if ($condition_options['header_name'] === '') {
							$context_errors[$condition_id][] = translate($this->prefix.'error_cond_header_no_header_name');
						}
						if ($condition_options['match_value'] === '') {
							$context_errors[$condition_id][] = translate($this->prefix.'error_cond_header_no_match_value');
						}
					break;
					
					case 'get':
					case 'post':
						if ($condition_options['var_name'] === '') {
							$context_errors[$condition_id][] = translate($this->prefix.'error_cond_no_request_var_name');
						}
						if ($condition_options['match_value'] === '') {
							$context_errors[$condition_id][] = translate($this->prefix.'error_cond_request_var_no_match_value');
						}
					break;
				}
			}
		}

		return $context_errors;

	}//end _getContextErrors()


	/**
	* Paints the edit interface for the currently selected context
	*
	* Note: Write access is not sent to this function since it is only displayed when
	*       you are able to edit a context, ie. write access must be TRUE.
	*
	* @param Backend_Outputter	$o					The backend outputter being used
	* @param array				$contexts			The list of currently available contexts
	*                                       		  (ie. SQ_CONTEXT_DEFINITIONS)
	* @param int				$current_context	The context being edited
	*
	* @return void
	*/
	public function paintSelectedContext(Backend_Outputter $o, Array $contexts, $current_context)
	{
		$o->openField(translate('name'));
			text_box($this->prefix.'name', array_get_index($contexts[$current_context], 'name', ''));
			hidden_field($this->prefix.'editing', (string)$current_context);
		$o->closeField();

		if ($current_context !== 0) {
			if (empty($contexts[$current_context]['at_least']) === FALSE) {
				$at_least = $contexts[$current_context]['at_least'];
			} else { 
				$at_least = NULL;
			}

			if (empty($contexts[$current_context]['conditions']) === FALSE) {
				$conditions = $contexts[$current_context]['conditions'];
			} else { 
				$conditions = Array();
			}

			$o->openField(translate($this->prefix.'minimum_matches'));
				$list_options = Array(
							'at_least'	=> translate('at_least'),
							'all'		=> translate('all'),
						   );
				
				if ($at_least === NULL) {
					$default_option = 'all';
					$at_least = '';
				} else {
					$default_option = 'at_least';
					if (is_numeric($at_least) === FALSE) {
						$at_least = '';
					}
				}

				ob_start();
					combo_box($this->prefix.'min', $list_options, FALSE, $default_option, 0, ' onchange="if (this.value == \'all\') document.getElementById(\''.$this->prefix.'at_least\').value = \'\';"');
				$cb = ob_get_clean();
				ob_start();
					int_text_box($this->prefix.'at_least', $at_least, FALSE, 3, 1, NULL, 2, FALSE, FALSE, ' onchange="if (this.value != \'\') document.getElementById(\''.$this->prefix.'minimum_matches\').selectedIndex = 0;');
				$tb = ob_get_clean();
				echo translate($this->prefix.'minimum_matches_interface', $cb, $tb);
				?>
				<?php
			$o->closeField();

			$o->openField(translate('conditions'));
				$this->_paintConditionsList($o, $conditions);
			$o->closeField();

			$option_types = Array(
								'accept-language'	=> translate($this->prefix.'type_accept-language'),
								'header'			=> translate($this->prefix.'type_header'),
								'url'				=> translate($this->prefix.'type_url'),
								'port'				=> translate($this->prefix.'type_port'),
								'secure'			=> translate($this->prefix.'type_secure'),
								'get'				=> translate($this->prefix.'type_get'),
								'post'				=> translate($this->prefix.'type_post'),
							);

			$o->openField(translate('new_condition_question'));
			?><p><?php
				ob_start();
					check_box($this->prefix.'new_condition', '1', FALSE, 'document.getElementById("'.$this->prefix.'new_condition_type").disabled = !this.checked; if (this.checked == true) { document.getElementById("'.$this->prefix.'new_condition_type").focus() };');
				$check_box = ob_get_clean();
				ob_start();
					combo_box($this->prefix.'new_condition_type', $option_types, FALSE, Array(), 0, ' disabled="disabled"');
				$combo_box = ob_get_clean();
				echo translate($this->prefix.'new_condition_line', $check_box, $combo_box);
			$o->closeField();
		}

	}//end paintSelectedContext()


	/**
	* Paints a table of conditions for an alternate context
	*
	* @param Backend_Outputter	$o			The outputter class for this editing interface
	* @param array				$conditions	The list of the conditions, in no
	*										particular order
	*
	* @return void
	*/
	protected function _paintConditionsList(Backend_Outputter $o, $conditions)
	{
		?>
	<table class="sq-backend-table" style="margin-bottom: 1em">
	<colgroup>
		<col width="10%" />
		<col />
		<col width="60" />
	</colgroup>
	<tbody>
	<tr class="sq-table-row">
		<th><?php echo translate($this->prefix.'condition_type'); ?></th>
		<th><?php echo translate($this->prefix.'condition_details'); ?></th>
		<th style="text-align: center"><?php echo translate('delete_question'); ?></th>
	</tr><?php
		foreach ($conditions as $condition_id => &$condition_options) {
		?>
		<tr><?php
			$condition_type = $conditions[$condition_id]['type'];
			?>
			<td><p><strong><?php echo translate($this->prefix.'type_'.$condition_type); ?></strong></p></td><?php
			?>
			<td><?php			
				$this->_paintCondition($o, $conditions, $condition_id);
			?></td>
			<td align="center"><p><strong><?php check_box($this->prefix.'condition_delete['.$condition_id.']', '1', FALSE); ?></strong></p></td><?php
		?>
		</tr><?php
		}
			
	?>
	</tbody>	
	</table>
		<?php

	}//end _paintConditionsList()


	/**
	* Paints a single condition's interface
	*
	* @param Backend_Outputter	$o				The outputter class for this editing interface
	* @param array				$conditions		The list of the conditions, in no
	*											particular order
	* @param int				$condition_id	The condition currently being painted
	*
	* @return void
	*/
	protected function _paintCondition(Backend_Outputter $o, $conditions, $condition_id)
	{
		$condition_type = $conditions[$condition_id]['type'];
		$condition_details = $conditions[$condition_id]['options'];

		switch ($condition_type) {
			case 'accept-language':
				$locales = $this->_getAcceptLanguageOptions();
				?>
				<ul>
				<li><?php
				$current_language = $condition_details['language'];
				ob_start();
					combo_box($this->prefix.$condition_id.'_language', $locales, FALSE, Array($current_language));
				$language_cb = ob_get_clean();
				echo translate($this->prefix.'type_accept-language_param_language', $language_cb);
				?></li>
				<li><?php
					ob_start();
						// Get current q-value and make sure it's in the bounds of 0.0..1.0
						$min_q = $condition_details['min_q'];
						$min_q = min($min_q, 1);
						$min_q = max($min_q, 0);

						// If 0 or 1, show it to the user with one decimal place
						// (ie. 0.0 or 1.0), similar to HTTP definition's examples.
						if ((float)$min_q == (int)$min_q) {
							$min_q = number_format($min_q, 1);
						}
						text_box($this->prefix.$condition_id.'_min_q', $min_q, 5, 0, FALSE, ' onblur="val = parseFloat(this.value); if (isNaN(val)) {this.value = \'\'; } else {val = Math.min(Math.max(val, 0), 1); if (val == parseInt(this.value)) {this.value = parseInt(this.value) + \'.0\'; } }"');
					$min_q_box = ob_get_clean();
					echo translate($this->prefix.'type_accept-language_param_min_q', $min_q_box);					
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_accept-language_param_explanation');
				?></p><?php
			break;

			case 'header':
				?>
				<ul>
				<li><?php
				$header_name    = $condition_details['header_name'];
				$match_logic    = $condition_details['match_logic'];
				$match_type     = $condition_details['match_type'];
				$match_value    = $condition_details['match_value'];
				$case_sensitive = $condition_details['case_sensitive'];
		
				$logic_options = Array(
									1	=> translate($this->prefix.'type_header_option_1'),
									0	=> translate($this->prefix.'type_header_option_0'),
							     );
		
				$type_options = Array(
									'exists'	=> translate($this->prefix.'type_header_option_exists'),
									'exact'		=> translate($this->prefix.'type_header_option_exact'),
									'begins'	=> translate($this->prefix.'type_header_option_begins'),
									'ends'		=> translate($this->prefix.'type_header_option_ends'),
									'contains'	=> translate($this->prefix.'type_header_option_contains'),
							     );

				ob_start();
					text_box($this->prefix.$condition_id.'_header_name', $header_name, 30);
				$header_tb = ob_get_clean();
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_logic', $logic_options, FALSE, Array($match_logic));
				$logic_cb = ob_get_clean();
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_type', $type_options, FALSE, Array($match_type));
				$type_cb = ob_get_clean();
				ob_start();
					text_box($this->prefix.$condition_id.'_match_value', $match_value, 30);
				$value_tb = ob_get_clean();
				ob_start();
					check_box($this->prefix.$condition_id.'_case_sensitive', '1', $case_sensitive);
				$case_ch = ob_get_clean();
				echo translate($this->prefix.'type_header_param', $header_tb, $logic_cb, $type_cb, $value_tb, $case_ch);
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_header_param_explanation');
				?></p><?php
			break;

			case 'get':
			case 'post':
				?>
				<ul>
				<li><?php
				$var_name    = $condition_details['var_name'];
				$match_logic    = $condition_details['match_logic'];
				$match_type     = $condition_details['match_type'];
				$match_value    = $condition_details['match_value'];
				$case_sensitive = $condition_details['case_sensitive'];
		
				$logic_options = Array(
									1	=> translate($this->prefix.'type_header_option_1'),
									0	=> translate($this->prefix.'type_header_option_0'),
							     );
		
				$type_options = Array(
									'exists'	=> translate($this->prefix.'type_header_option_exists'),
									'exact'		=> translate($this->prefix.'type_header_option_exact'),
									'begins'	=> translate($this->prefix.'type_header_option_begins'),
									'ends'		=> translate($this->prefix.'type_header_option_ends'),
									'contains'	=> translate($this->prefix.'type_header_option_contains'),
							     );

				ob_start();
					text_box($this->prefix.$condition_id.'_var_name', $var_name, 30);
				$var_tb = ob_get_clean();
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_logic', $logic_options, FALSE, Array($match_logic));
				$logic_cb = ob_get_clean();
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_type', $type_options, FALSE, Array($match_type));
				$type_cb = ob_get_clean();
				ob_start();
					text_box($this->prefix.$condition_id.'_match_value', $match_value, 30);
				$value_tb = ob_get_clean();
				ob_start();
					check_box($this->prefix.$condition_id.'_case_sensitive', '1', $case_sensitive);
				$case_ch = ob_get_clean();
				echo translate($this->prefix.'type_'.$condition_type.'_param', $var_tb, $logic_cb, $type_cb, $value_tb, $case_ch);
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_header_param_explanation');
				?></p><?php
			break;

			case 'url':
				?>
				<ul>
				<li><?php
				$match_logic = $condition_details['match_logic'];
				$match_type  = $condition_details['match_type'];
				$match_value = $condition_details['match_value'];
		
				$logic_options = Array(
									1	=> translate($this->prefix.'type_url_option_1'),
									0	=> translate($this->prefix.'type_url_option_0'),
							     );
		
				$type_options = Array(
									'exact'		=> translate($this->prefix.'type_url_option_exact'),
									'begins'	=> translate($this->prefix.'type_url_option_begins'),
									'ends'		=> translate($this->prefix.'type_url_option_ends'),
									'contains'	=> translate($this->prefix.'type_url_option_contains'),
							     );

				ob_start();
					combo_box($this->prefix.$condition_id.'_match_logic', $logic_options, FALSE, Array($match_logic));
				$logic_cb = ob_get_clean();
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_type', $type_options, FALSE, Array($match_type));
				$type_cb = ob_get_clean();
				ob_start();
					text_box($this->prefix.$condition_id.'_match_value', $match_value, 30);
				$value_tb = ob_get_clean();
				echo translate($this->prefix.'type_url_param', $logic_cb, $type_cb, $value_tb);
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_url_param_explanation');
				?></p><?php
			break;

			case 'port':
				?>
				<ul>
				<li><?php
				$match_type = $condition_details['match_type'];
				$port = $condition_details['match_value'];
				$options = Array(
							'is'		=> translate($this->prefix.'type_port_option_1'),
							'is_not'	=> translate($this->prefix.'type_port_option_0'),
						   );
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_type', $options, FALSE, Array($match_type));
				$cb = ob_get_clean();
				ob_start();
					int_text_box($this->prefix.$condition_id.'_match_value', $port, FALSE, 7, 0, 65535);
				$tb = ob_get_clean();
				echo translate($this->prefix.'type_port_param', $cb, $tb);
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_port_param_explanation');
				?></p><?php
			break;

			case 'secure':
				?>
				<ul>
				<li><?php
				$match = $condition_details['match_value'];
				$options = Array(
							0	=> translate($this->prefix.'type_secure_option_0'),
							1	=> translate($this->prefix.'type_secure_option_1'),
						   );
				ob_start();
					combo_box($this->prefix.$condition_id.'_match_value', $options, FALSE, Array($match));
				$cb = ob_get_clean();
				echo translate($this->prefix.'type_secure_param', $cb);
				?>
				</li>
				</ul>
				<p><?php
					echo translate($this->prefix.'type_secure_param_explanation');
				?></p><?php
			break;
		}

	}//end _paintCondition()


	/**
	* Processes the edit interface for the currently selected context
	*
	* @param Backend_Outputter	$o					The backend outputter being used
	* @param array				$contexts			The list of currently available contexts
	*                                       		  (ie. SQ_CONTEXT_DEFINITIONS)
	* @param int				$current_context	The context being edited
	*
	* @return void
	*/
	public function processSelectedContext(Backend_Outputter $o, Array $contexts, $current_context)
	{

	}//end processSelectedContext()

	
	/**
	* Paints the edit interface for changing the context matching order
	*
	* @param Backend_Outputter	$o				The backend outputter being used
	* @param array				$match_order	The current matching order list
	*                                       	  (ie. SQ_CONTEXT_MATCH_ORDER)
	* @param boolean			$write_access	TRUE if write access available
	*
	* @return void
	*/
	public function paintMatchOrder(Backend_Outputter $o, Array $match_order, $write_access)
	{

	}//end paintMatchOrder()


	/**
	* Processes the edit interface for changing the context matching order
	*
	* @param Backend_Outputter	$o					The backend outputter being used
	* @param array				$match_order		The current matching order list
	*                                       		  (ie. SQ_CONTEXT_MATCH_ORDER)
	*
	* @return void
	*/
	public function processMatchOrder(Backend_Outputter $o, Array $match_order)
	{

	}//end processMatchOrder()


	/**
	* Process the entire screen
	*
	* @param Backend_Outputter	$o					The backend
	* @param array				$match_order		The current matching order list
	*                                       		  (ie. SQ_CONTEXT_MATCH_ORDER)
	*
	* @return void
	*/
	public function processBackend(Backend_Outputter $o)
	{
		if (empty($_POST['sq_lock_release_manual']) === FALSE) {
			$this->releaseLock();
		}

		$contexts = $this->load();
		$context_names = Array();
		foreach ($contexts as $key => $value){
			$context_names[] = $value['name'];
		}
		
		$save_required = FALSE;
		$saved = FALSE;

		$current_context = NULL;
		$editing_context = NULL;
		$new_key = NULL;

		// Don't release the lock unless we tell you to
		unset($_POST['sq_lock_release']);
		
		if ($this->writeAccess() === TRUE) {
			$class = get_class_lower($this);
			
			// Post variables
			$deleted_context = array_get_index($_POST, $this->prefix.'deleted');
			if (is_numeric($deleted_context) === TRUE) {
				// Deleting a context outright
				unset($contexts[$deleted_context]);
				$active_context = $GLOBALS['SQ_SYSTEM']->getContextId();
				if ((int)$deleted_context === (int)$active_context) {
					$GLOBALS['SQ_SYSTEM']->restoreContext();
					$new_alternate_context = $GLOBALS['SQ_SYSTEM']->getAlternateContext();
					$GLOBALS['SQ_SYSTEM']->changeContext($new_alternate_context);
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $new_alternate_context;
				}
				$save_required = TRUE;

				// Release the lock
				$_POST['sq_lock_release'] = TRUE;
			} else {
				// Post variables
				$current_context = array_get_index($_POST, $this->prefix.'selected');
				if (!is_numeric($current_context)) {
					// Don't let bad data get the better of this screen
					$current_context = NULL;
				} else {
					$current_context = (int)$current_context;
				}
				
				if ($current_context === NULL) {
					// We can only be editing a context if we haven't selected one
					// in the same page request.
					$editing_context = array_get_index($_POST, $this->prefix.'editing');
					if (!is_numeric($editing_context)) {
						$editing_context = NULL;
					} else {
						$editing_context = (int)$editing_context;
					}
				} else {
					$editing_context = NULL;
				}

				// Have the contexts been reordered in the interface?
				$reorder_post = array_get_index($_POST, $this->prefix.'reorder', Array());
				if (empty($reorder_post) === FALSE) {
					foreach ($reorder_post as $new_sort_order => $order_contextid) {
						if ((int)$contexts[$order_contextid]['sort_order'] !== (int)$new_sort_order) {
							$contexts[$order_contextid]['sort_order'] = $new_sort_order;
							$save_required = TRUE;
						}
					}
				}

				// This screen doesn't fulfil the usual rules for config saving because
				// of its complexity - usually we need POST vars like
				// $_POST['context_config']['SQ_CONTEXT_DEFINITIONS'] or similar.
				// So build them here before letting parent process the backend.

				if ($current_context === NULL) {
				if ((int)array_get_index($_POST, $this->prefix.'new_alternate_context', '0') === 1) {
					$new_name = trim(array_get_index($_POST, $this->prefix.'new_alternate_context_name', ''));

					if ($new_name === '' || in_array($new_name, $context_names)) {
						trigger_localised_error('CTX0005', E_USER_WARNING);
					} else {
						// Creating a new context - include the new name
						$new_key = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ctx_id_seq'));
						$new_sort_order = count($contexts);
						$contexts[$new_key] = Array(
												'name'			=> array_get_index($_POST, $this->prefix.'new_alternate_context_name', ''),
												'conditions'	=> Array(),
												'at_least'		=> NULL,
												'sort_order'	=> $new_sort_order,
											  );

						// Allow the new key to flow through to the editing interface,
						// so the user can edit the new context
						$_POST[$this->prefix.'selected'] = $new_key;
						$save_required = TRUE;
					}
				}
				}
				
				if (($current_context === NULL) && ($editing_context !== NULL)) {
					// Editing a currently selected context
					$new_name = trim(array_get_index($_POST, $this->prefix.'name', ''));

					// If the new name is blank or already exists, refuse to change it, but allow
					// the rest of the editing to continue
					if ($new_name === '' || ($new_name != $contexts[$editing_context]['name'] && in_array($new_name, $context_names))) {
						trigger_localised_error('CTX0006', E_USER_NOTICE, $contexts[$editing_context]['name']);
					} else {
						$contexts[$editing_context]['name'] = $new_name;
					}

					if ($editing_context !== 0) {
						if (array_get_index($_POST, $this->prefix.'min') === 'all') {
							$at_least = NULL;
						} else {
							$at_least = array_get_index($_POST, $this->prefix.'at_least', 1);
						}
						$contexts[$editing_context]['at_least'] = $at_least;

						$deleted_conditions = array_get_index($_POST, $this->prefix.'condition_delete', Array());
						foreach ($deleted_conditions as $condition_id => $value) {
							unset($contexts[$editing_context]['conditions'][$condition_id]);
						}

						foreach ($contexts[$editing_context]['conditions'] as $condition_id => &$condition_value) {
							$this->_processCondition($condition_id, $condition_value['type'], $condition_value['options']);
						}

						// We are adding a new condition
						$new_condition_flag = array_get_index($_POST, $this->prefix.'new_condition', '0');
						if ((bool)$new_condition_flag === TRUE) {
							$new_condition_type = array_get_index($_POST, $this->prefix.'new_condition_type');
							$new_condition = Array(
												'type'	=> $new_condition_type,
											 );
							
							switch ($new_condition_type) {
								case 'accept-language':
									// Defaults for accept-language header:
									// - Language not selected (REQUIRED to be valid)
									// - Minimum q-value of 0.0 (ie. language will match at any quality)
									$new_condition['options'] = Array(
																	'language'	=> 'en',
																	'min_q'		=> 0,
																);
								break;

								case 'header':
									// Defaults for 'other' header:
									// - Header name not specified (REQUIRED to be valid)
									// - Exact match to an empty string
									$new_condition['options'] = Array(
																	'header_name'	=> '',
																	'match_logic'	=> '1',
																	'match_type'	=> 'exact',
																	'match_value'	=> '',
																	'case_sensitive'	=> '1',
																);
								break;

								case 'get':
								case 'post':
									// Defaults for 'other' header:
									// - Header name not specified (REQUIRED to be valid)
									// - Exact match to an empty string
									$new_condition['options'] = Array(
																	'var_name'	=> '',
																	'match_logic'	=> '1',
																	'match_type'	=> 'exact',
																	'match_value'	=> '',
																	'case_sensitive'	=> '1',
																);
								break;

								case 'url':
									// Defaults for match by URL:
									// - Match value not specified (REQUIRED to be valid)
									// - Exact match by default
									$new_condition['options'] = Array(
																	'match_logic'	=> '1',
																	'match_type'	=> 'exact',
																	'match_value'	=> '',
																);
								break;

								case 'port':
									// Match on port defaults to matching on port 80
									$new_condition['options'] = Array(
																	'match_type'	=> 'is',
																	'match_value'	=> 80,
																);
								break;

								case 'secure':
									// Match on secure: defaults to force secure 
									$new_condition['options'] = Array(
																	'match_value'	=> 1,
																);
								break;

							}//end switch on condition type

							$contexts[$editing_context]['conditions'][] = $new_condition;

						}//end if new condition is being created
					
					}//end if context being edited is not default

					$save_required = TRUE;
				
				}//end if editing context is not null

			}//end if deleted context is not null

		
			// This page cannot work properly with the default behaviour
			// of redirecting on save (usually done since define()'d constants
			// cannot be redefined). So we save a copy of the new vars
			// in save() and re-use them in paintBackend().
			//
			// The copied code (from parent::processBackend()) exists so we can
			// still give a correct TRUE/FALSE indication from save(), since 
			// this normally determines whether it redirects or not.
			if (empty($_POST['process_form']) === FALSE) {
				if ($save_required === TRUE) {
					$vars = Array(
								'contexts'	=> $contexts,
							);
					$saved = $this->save($vars);
				}
			}
		}//end if write access permitted

		if (empty($_POST['sq_lock_release']) === FALSE) {
			$this->releaseLock();
		}

		// If we still have a lock, refresh it
		$this->updateLock();

		// if there is no lock currently and we want it
		if (empty($_POST['sq_lock_acquire']) === FALSE) {
			$this->acquireLock();
		}

		// If we are in the middle of editing a context, ensure it is still
		// selected - unless we created a new alternate context
		if (($editing_context !== NULL) && ($current_context === NULL) && ($new_key === NULL)) {
			$_POST[$this->prefix.'selected'] = $editing_context;
		}

		return $saved;

	}//end processBackend()


	/**
	* Process a single condition
	*
	* @param int	$condition_id		The ID of the condition being processed -
	*									used mainly for POST vars
	* @param string	$condition_type		The type of condition we are processing
	* @param array	&$condition_options	Options for this condition - can be
	*									processed in place
	*
	* @return void
	*/
	protected function _processCondition($condition_id, $condition_type, &$condition_options)
	{
		switch ($condition_type) {
			case 'accept-language':
				$locales = $this->_getAcceptLanguageOptions();
				$new_language = array_get_index($_POST, $this->prefix.$condition_id.'_language', '');
				if (isset($locales[$new_language]) === FALSE) {
					trigger_localised_error('CTX0008', E_USER_NOTICE, $new_language);
				} else {
					$condition_options['language'] = $new_language;
				}

				$new_q_value = array_get_index($_POST, $this->prefix.$condition_id.'_min_q');
				if (is_numeric($new_q_value) === TRUE) {
					$new_q_value = (float)$new_q_value;
					if ($new_q_value > 1.0) {
						trigger_localised_error('CTX0009', E_USER_NOTICE, $new_q_value);
						$new_q_value = 1.0;
					}
					if ($new_q_value < 0.0) {
						trigger_localised_error('CTX0010', E_USER_NOTICE, $new_q_value);
						$new_q_value = 0.0;
					}
					$condition_options['min_q'] = $new_q_value;
				}

			break;

			case 'header':
				$condition_options['header_name'] = array_get_index($_POST, $this->prefix.$condition_id.'_header_name', '');
				$condition_options['case_sensitive'] = array_get_index($_POST, $this->prefix.$condition_id.'_case_sensitive', 0);
				$condition_options['match_logic'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_logic', 1);
				$condition_options['match_type']  = array_get_index($_POST, $this->prefix.$condition_id.'_match_type',  'exact');
				$condition_options['match_value'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_value', ''); 
			break;

			case 'get':
			case 'post':
				$condition_options['var_name'] = array_get_index($_POST, $this->prefix.$condition_id.'_var_name', '');
				$condition_options['case_sensitive'] = array_get_index($_POST, $this->prefix.$condition_id.'_case_sensitive', 0);
				$condition_options['match_logic'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_logic', 1);
				$condition_options['match_type']  = array_get_index($_POST, $this->prefix.$condition_id.'_match_type',  'exact');
				$condition_options['match_value'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_value', ''); 
			break;
			
			case 'url':
				$condition_options['match_logic'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_logic', 1);
				$condition_options['match_type']  = array_get_index($_POST, $this->prefix.$condition_id.'_match_type',  'exact');
				$condition_options['match_value'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_value', ''); 
			break;

			case 'port':
				$condition_options['match_type'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_type', 1);
				$condition_options['match_value'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_value', 80);
			break;

			case 'secure':
				$condition_options['match_value'] = array_get_index($_POST, $this->prefix.$condition_id.'_match_value', 0);
			break;
			
		}//end switch on condition type

	}//end _processCondition()


	/**
	* Load contexts from database
	*
	* @return array
	*/
	public function load()
	{
		$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts(TRUE);

		// Remove the intervening '0' since the context ID is unique;
		// also unserialise conditions where needed
		// use PHP 5 assign-value-by-ref trick
		if (isset($contexts[0]) === FALSE) {
			$contexts[0] = Array(
							'name'			=> 'Default Context',
							'at_least'		=> NULL,
							'conditions'	=> NULL,
							'sort_order'	=> NULL,
						   );
		}

		return $contexts;

	}//end load()


	/**
	* Rewrites the configuration with current variables
	*
	* This needs to be completely overridden since this config will
	* save to DB rather than to config file. Since we control
	* processBackend() here, this is completely fine.
	*
	* Returns TRUE on successful write to DB, FALSE otherwise.
	*
	* @param array		$vars				an array with the new set of variables
	*										Array(string => mixed)
	* @param boolean	$backup_existing	Ignored here as we don't have a config file
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE)
	{
		if ($this->writeAccess() === FALSE) {
			return FALSE;
		}

		// Cheat method to emulate getCol()
		$existing_contextids = array_keys(MatrixDAL::executeGrouped('core', 'getAllContextIDs', Array()));
		$current_contextids = array_keys(array_get_index($vars, 'contexts', Array()));

		$new_contextids = array_diff($current_contextids, $existing_contextids);
		$deleted_contextids = array_diff($existing_contextids, $current_contextids);
		$updated_contextids = array_intersect($current_contextids, $existing_contextids);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');		
	
		$ok = TRUE;

		foreach ($new_contextids as $new_contextid) {
			try {
				$context =& $vars['contexts'][$new_contextid];
				$bind_vars = Array(
								'contextid'		=> $new_contextid,
								'name'			=> $context['name'],
								'conditions'	=> serialize($context['conditions']),
								'at_least'		=> $context['at_least'],
								'sort_order'	=> $context['sort_order'],
							 );
				MatrixDAL::executeQuery('core', 'createContext', $bind_vars);

			} catch (DALException $e) {
				// Cannot insert context due to DB error.
				// It's not fatal from a PHP point of view - it is for
				// this processing though, so send a warning and break out
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_localised_error('CTX0001', E_USER_ERROR, $bind_vars['name'], $e->getMessage());
				exit(1);

			}//end try

		}//end foreach - new contexts

		if ($ok === TRUE) {
			foreach ($updated_contextids as $updated_contextid) {
				try {
					$context =& $vars['contexts'][$updated_contextid];
					$bind_vars = Array(
									'contextid'		=> $updated_contextid,
									'name'			=> $context['name'],
									'conditions'	=> serialize($context['conditions']),
									'at_least'		=> $context['at_least'],
									'sort_order'	=> $context['sort_order'],
								 );
					MatrixDAL::executeQuery('core', 'updateContext', $bind_vars);

				} catch (DALException $e) {
					// Cannot update context due to DB error.
					// It's not fatal from a PHP point of view - it is for
					// this processing though, so send a warning and break out
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					trigger_localised_error('CTX0002', E_USER_ERROR, $bind_vars['name'], $e->getMessage());
					exit(1);

				}//end try

			}//end foreach - updated contexts

		}//end if

		// delete the condemned contexts all in one go
		if (($ok === TRUE) && (count($deleted_contextids) > 0)) {
			try {
				$bind_vars = Array(
								'contextids'	=> $deleted_contextids,
							 );
				MatrixDAL::executeQuery('core', 'deleteContexts', $bind_vars);

			} catch (DALException $e) {
				// Cannot delete context(s) due to DB error,
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				if (count($deleted_contextids) === 1) {
					$deleted_contextid = $deleted_contextids[0];
					trigger_localised_error('CTX0003', E_USER_ERROR, $deleted_contextid, $e->getMessage());
				} else {
					trigger_localised_error('CTX0004', E_USER_ERROR, count($deleted_contextids), $e->getMessage());
				}
				exit(1);
				
			}//end try

		}//end if

		foreach ($new_contextids as $new_contextid) {
			if ($GLOBALS['SQ_SYSTEM']->insertContextData($new_contextid) === FALSE) {
				// ??? Presuming here that above function will throw error, perhaps fatal
				exit(1);
			}
		}

		foreach ($deleted_contextids as $deleted_contextid) {
			if ($GLOBALS['SQ_SYSTEM']->deleteContextData($deleted_contextid) === FALSE) {
				// ??? Presuming here that above function will throw error, perhaps fatal
				exit(1);
			}
		}

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $ok;

	}//end save()


	/**
	* Get Accept-Language options
	*
	* @return array
	*/
	protected function _getAcceptLanguageOptions()
	{
		static $options = NULL;

		if (empty($options) === TRUE) {
			include SQ_FUDGE_PATH.'/standards_lists/languages.inc';
			include SQ_FUDGE_PATH.'/standards_lists/locales.inc';
			// Remove locales that are variations of other ones (eg.
			// for Euro support), often signified by a "@"
			foreach (array_keys($standards_lists_locales) as $locale_code) {
				$matches = preg_match('/^([a-zA-Z\_\-]+)$/', $locale_code);
				if ($matches === 0) {
					unset($standards_lists_locales[$locale_code]);
				}			
			}
			$overlap = array_intersect($standards_lists_languages, $standards_lists_locales);

			foreach ($overlap as $code => $language) {
				if(isset($standards_lists_languages[$code])) {
					$standards_lists_languages[$code] = translate('non_localised_variation', $language);
				}
			}

			$options = $standards_lists_locales + $standards_lists_languages;
			ksort($options);

		}

		return $options;

	}//end _getAcceptLanguageOptions()

}//end class

?>
