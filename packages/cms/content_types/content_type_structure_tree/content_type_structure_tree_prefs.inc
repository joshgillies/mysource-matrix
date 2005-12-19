<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_structure_tree_prefs.inc,v 1.2 2005/12/19 05:21:58 dmckee Exp $
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
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Content_Type_Structure_Tree_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_STRUCTURE_TREE_FORMATTING'		=> Array(
																'name'			=> 'Structure Tree Format Options',
																'description'	=> 'This preference allows you to change the text options that are displayed',
																'default'		=> Array(
																					'bold'			=> true,
																					'underline'		=> true,
																					'italic'		=> true,
																					'delete'		=> true,
																					'insert'		=> true,
																					'connections'	=> true,
																				   ),
																'protected'		=> false,
															   ),
						'SQ_STRUCTURE_TREE_NODE_CLASSES'	=> Array(
																'name'			=> 'Structure Tree CSS Class',
																'description'	=> 'This preference allows you to change the class used for each Node',
																'default'		=> Array(
																					'node_text'		=> 'tree_node_text',
																					'node_branches'	=> 'tree_node_branches',
																				   ),
																'protected'		=> false,
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
	function paintBackend(&$o, $have_lock, $pref=null)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_FORMATTING') {
			if (is_null($pref)) {
				$o->openField(translate('structure_tree_title'));
			}
				$format_options = $this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('tree_formatting_options'); ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							check_box('SQ_STRUCTURE_SHOW_BOLD', '1', $format_options['bold']);
							echo translate('tree_bold').'<br />';
							check_box('SQ_STRUCTURE_SHOW_UNDERLINE', '1', $format_options['underline']);
							echo translate('tree_underline').'<br />';
							check_box('SQ_STRUCTURE_SHOW_ITALIC', '1', $format_options['italic']);
							echo translate('tree_italic').'<br />';
							check_box('SQ_STRUCTURE_SHOW_DELETE', '1', $format_options['delete']);
							echo translate('delete').'<br />';
							check_box('SQ_STRUCTURE_SHOW_INSERT', '1', $format_options['delete']);
							echo translate('insert').'<br />';
							check_box('SQ_STRUCTURE_SHOW_CONNECTIONS', '1', $format_options['connections']);
							echo translate('tree_connections').'<br />';
						} else {
							$tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
							$cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="Yes" />';
							?>
							<table>
								<tr>
									<td>
										<?php echo translate('tree_bold'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['bold'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('tree_underline'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['underline'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('tree_italic'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['italic'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('delete'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['delete'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('insert'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['insert'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
								<tr>
									<td>
										<?php echo translate('tree_connections'); ?>&nbsp;
									</td>
									<td>
										<?php
										echo ($format_options['connections'] == true) ? $tick : $cross;
										?>
									</td>
								</tr>
							</table>
							<?php
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			if (is_null($pref)) $o->closeField();
		}
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_NODE_CLASSES') {
			if (is_null($pref)) {
				$o->openField(translate('tree_class_title'));
			}
				$classes = $this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
				<?php
					foreach ($classes as $name => $class) {
				?>
						<tr>
							<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate($name); ?></td>
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
		}

		return true;

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
	function processBackend(&$o, $have_lock, $pref=null)
	{
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['bold'] = isset($_POST['SQ_STRUCTURE_SHOW_BOLD']);
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['underline'] = isset($_POST['SQ_STRUCTURE_SHOW_UNDERLINE']);
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['italic'] = isset($_POST['SQ_STRUCTURE_SHOW_ITALIC']);
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['delete'] = isset($_POST['SQ_STRUCTURE_SHOW_DELETE']);
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['insert'] = isset($_POST['SQ_STRUCTURE_SHOW_INSERT']);
		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['connections'] = isset($_POST['SQ_STRUCTURE_SHOW_CONNECTIONS']);
		$this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['default'] = $_POST['SQ_STRUCTURE_TREE_NODE_CLASSES'];
		return true;
	}//end processBackend()


}//end class

?>