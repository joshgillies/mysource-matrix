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
* $Id: content_type_structure_tree_prefs.inc,v 1.8 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Structure_Tree_Prefs
*
* Purpose
*
*    Looks after the creation the Structure Tree
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/
class Content_Type_Structure_Tree_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_STRUCTURE_TREE_FORMATTING'		=> Array(
																'name'			=> 'structure_tree_title',
																'description'	=> 'This preference allows you to change the text options that are displayed',
																'default'		=> Array(
																					'bold'			=> TRUE,
																					'underline'		=> TRUE,
																					'italic'		=> TRUE,
																					'delete'		=> TRUE,
																					'insert'		=> TRUE,
																					'connections'	=> TRUE,
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_STRUCTURE_TREE_NODE_CLASSES'	=> Array(
																'name'			=> 'tree_class_title',
																'description'	=> 'This preference allows you to change the class used for each Node',
																'default'		=> Array(
																					'node_text'		=> 'tree_node_text',
																					'node_branches'	=> 'tree_node_branches',
																				   ),
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_Structure_Tree_Prefs($pref_file='')
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
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_FORMATTING') {
			if (is_null($pref)) {
				$o->openField(translate('Structured Tree Formatting Options'));
			}
				$format_options = $this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Formatting Options to Show'); ?></td>

						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							check_box('SQ_STRUCTURE_SHOW_BOLD', '1', $format_options['bold']);
							echo translate('Bold').'<br />';

							check_box('SQ_STRUCTURE_SHOW_UNDERLINE', '1', $format_options['underline']);
							echo translate('Underline').'<br />';

							check_box('SQ_STRUCTURE_SHOW_ITALIC', '1', $format_options['italic']);
							echo translate('Italic').'<br />';

							check_box('SQ_STRUCTURE_SHOW_DELETE', '1', $format_options['delete']);
							echo translate('Delete').'<br />';

							check_box('SQ_STRUCTURE_SHOW_INSERT', '1', $format_options['delete']);
							echo translate('Insert').'<br />';

							check_box('SQ_STRUCTURE_SHOW_CONNECTIONS', '1', $format_options['connections']);
							echo translate('Connections').'<br />';

						} else {
							$tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
							$cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="Yes" />';
							?>
							<table>
								<tr>
									<td>
										<?php echo translate('Bold'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['bold'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('Underline'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['underline'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('Italic'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['italic'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('Delete'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['delete'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('Insert'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['insert'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('Connections'); ?>&nbsp;

									</td>
									<td>
										<?php
										echo ($format_options['connections'] == TRUE) ? $tick : $cross;
										?>
									</td>
								</tr>
							</table>
							<?php
						}//end else
						?>
						</td>
					</tr>
				</table>
				<?php
			if (is_null($pref)) $o->closeField();
		}//end if
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_NODE_CLASSES') {
			if (is_null($pref)) {
				$o->openField(translate('Structured Tree CSS Classes'));
			}
				$classes = $this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
				<?php
				    $class_text = Array(
				                   'node_text'     => translate('Node Text'),
				                   'node_branches' => translate('Node Branches'),
				                  );
					foreach ($classes as $name => $class) {
				?>
						<tr>
							<td class="sq-backend-table-header" nowrap="nowrap"><?php echo $class_text[$name]; ?></td>
							<td class="sq-backend-table-cell" style="width: 100%;">
							<?php
								if ($have_lock && $is_admin) {
									text_box('SQ_STRUCTURE_TREE_NODE_CLASSES['.$name.']', $class, '20');
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
			if (is_null($pref)) $o->closeField();
		}//end if

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

		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_FORMATTING') {
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['bold'] = isset($_POST['SQ_STRUCTURE_SHOW_BOLD']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['underline'] = isset($_POST['SQ_STRUCTURE_SHOW_UNDERLINE']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['italic'] = isset($_POST['SQ_STRUCTURE_SHOW_ITALIC']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['delete'] = isset($_POST['SQ_STRUCTURE_SHOW_DELETE']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['insert'] = isset($_POST['SQ_STRUCTURE_SHOW_INSERT']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['connections'] = isset($_POST['SQ_STRUCTURE_SHOW_CONNECTIONS']);
		}
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_NODE_CLASSES') {
			$this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['default'] = $_POST['SQ_STRUCTURE_TREE_NODE_CLASSES'];
		}
		return TRUE;

	}//end processBackend()


}//end class

?>