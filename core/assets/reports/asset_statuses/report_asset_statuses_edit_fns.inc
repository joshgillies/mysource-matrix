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
* $Id: report_asset_statuses_edit_fns.inc,v 1.1 2005/02/16 06:02:14 dbaranovskiy Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Report_Asset_Statuses_Edit_Fns
*
* Edit Functions for the backend interface of asset statuses report
*
*
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report_Asset_Statuses_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Report_Asset_Statuses_Edit_Fns()
	{
		$this->Report_Edit_Fns();

	}//end constructor


	/**
	* Generate this report until complete
	*
	* @param object HIPO_Job	&$job		a reference to the array of information about
	* @param array				&$step_data	a reference to the array of information about
	*										the current step of the HIPO job that is running this report
	* @param string				$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$owner = &$am->getAsset($job->_running_vars['assetid'], 'report_asset_counter');
		$where = Array('a.status_changed_userid = u.assetid');
		$tables = Array('ast a', 'ast u');

		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, '', false, 'root');
		if (!empty($root_link)) {
			// need to restrict results according to link tree position too
			$parent_treeid = $db->getOne('SELECT t.treeid FROM '.SQ_TABLE_PREFIX.'ast_lnk_tree WHERE linkid = '.$db->quote($root_link['linkid']));
			$where[] = 'SUBSTRING(t.treeid FROM 1 FOR '.strlen($parent_treeid).') = '.$db-quote($parent_treeid);
			$tables[] = 'ast_lnk l';
			$tables[] = 'ast_lnk_tree t';
			$where[] = 'l.linkid = t.linkid';
			$where[] = 'a.assetid = l.minorid';
		}


		$search_status = $owner->attr('search_status');
		if ($search_status != SQ_SC_STATUS_ALL) {
			$where[] = 'a.status = '.$db->quote($search_status);
		}

		$period_length = $owner->attr('status_change_period');
		if ($period_length) {
			$operator = $owner->attr('status_change_comparator_more_than') ? '<' : '>';
			$timestamp = time() - ($period_length * 60);
			$where[] = 'a.status_changed '.$operator.' '.$db->quote(ts_iso8601($timestamp));
		}

		foreach ($tables as $index => $table) {
			$tables[$index] = SQ_TABLE_PREFIX.$table;
		}
		$sql = 'SELECT a.assetid, a.type_code, a.name, a.status, a.status_changed, a.status_changed_userid, u.name as user_name FROM '.implode(', ', $tables).' WHERE ('.implode(') AND (', $where).')';

		$result = $db->getAll($sql);

		// generate XML contents
		$contents = '<asset_list>'."\n";
		foreach ($result as $values) {
			$contents .= '<asset assetid="'.$values['assetid'].'" name="'.$values['name'].'" type_code="'.$values['type_code'].'">'."\n";
			$contents .= '	<status value="'.$values['status'].'" changed="'.$values['status_changed'].'" changed_userid="'.$values['status_changed_userid'].'" changed_user_name="'.$values['user_name'].'"/>'."\n";
			$contents .= '</asset>'."\n";
		}
		$contents .= '</asset_list>';



		$report_dir = $owner->data_path;
		$xml_file_name = $report_dir.'/report.xml';

		if (!is_dir($report_dir)) {
			if (!create_directory($report_dir)) {
				trigger_error('Could not create data directory, report generation aborted', E_USER_WARNING);
				return false;
			}
		}

		// file manipulation section
		$file_handler = fopen($xml_file_name, 'w');
		if ($file_handler === false) {
			trigger_error('Could not open report file, report generation aborted', E_USER_WARNING);
			return false;
		}
		fwrite($file_handler, $contents);
		fclose($file_handler);

		$step_data['percent_done'] = 100;
		$step_data['complete']     = true;
		return true;

	}//end generateReport()


	/**
	* Paints the field for choosing the period of last status change
	*
	* @param object	Asset_Listing		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintLastStatusChange(&$asset, &$o, $prefix)
	{
		$minute_count = $asset->attr('status_change_period');
		$days = (int)($minute_count / (60 * 24));
		$hours = (int)(($minute_count / 60) - ($days * 24));
		$minutes = $minute_count % 60;

		if ($asset->writeAccess('links')) {
			combo_box($prefix.'_status_change_comparator_more_than', Array('1' => 'more', '0' => 'less'), false, $asset->attr('status_change_comparator_more_than'));
			echo ' than ';
			text_box($prefix.'_days', $days, 1, 3, false, 'style="width: 2em"');
			echo ' days, ';
			text_box($prefix.'_hours', $hours, 1, 3, false, 'style="width: 2em"');
			echo ' hours and ';
			text_box($prefix.'_minutes', $minutes, 1, 3, false, 'style="width: 2em"');
			echo ' minutes ago ';
			return true;
		} else {
			echo $asset->attr('status_change_comparator_more_than') ? 'more' : 'less';
			echo ' than '.$days.' days, '.$hours.' hours and '.$minutes.' minutes ago';
		}

	}//end paintLastStatusChange()


	/**
	* Process that last status change time
	*
	* @param object	Asset_Listing		&$asset		asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processLastStatusChange(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_days'])) {
			$minutes_count = ($_POST[$prefix.'_days'] * 24 * 60) + ($_POST[$prefix.'_hours'] * 60) + $_POST[$prefix.'_minutes'];
			$asset->setAttrValue('status_change_period', $minutes_count);
		}
		if (isset($_POST[$prefix.'_status_change_comparator_more_than'])) {
			$asset->setAttrValue('status_change_comparator_more_than', $_POST[$prefix.'_status_change_comparator_more_than']);
		}

	}//end processLastStatusChange()


	/**
	* Paint the interface for viewing a report
	*
	* @param object Report				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$report_path = $asset->data_path.'/report.xml';

		if (!is_file($report_path)) {
			echo 'This report has not been generated';
			return;
		}

		$input = new XML_Tree($report_path);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Unable to read report file : '.$report_path."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" style="width: 10ex">Icon</td>
				<td class="sq-backend-table-header" style="width: 10ex">Asset ID</td>
				<td class="sq-backend-table-header">Name</td>
				<td class="sq-backend-table-header">Status</td>
				<td class="sq-backend-table-header">Last Status Change</td>
				<td class="sq-backend-table-header">Last Status Changer</td>
			</tr>
		<?php
		foreach ($root->children as $asset_node) {
			$assetid = $asset_node->attributes['assetid'];
			$name = $asset_node->attributes['name'];
			$type_code = $asset_node->attributes['type_code'];
			$type_icon = $am->getAssetIconURL($type_code);

			$status = $asset_node->children[0]->attributes['value'];
			$status_changed = $asset_node->children[0]->attributes['changed'];
			$status_changed_userid = $asset_node->children[0]->attributes['changed_userid'];
			$status_changed_user_name = $asset_node->children[0]->attributes['changed_user_name'];

			$status_names = get_status_names();

			?>
				<tr>
					<td class="sq-backend-table-cell">
						<script type="text/javascript">sq_print_icon("<?php echo $type_icon; ?>", "16", "16", "");</script>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $assetid; ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $name; ?>
					</td>
					<td class="sq-backend-table-cell">
						<span style="padding: 0 1ex 0 1ex;background: #<?php echo get_status_colour($status); ?>"><?php echo $status_names[$status]; ?></span>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $status_changed; ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $status_changed_user_name; ?> [<?php echo $status_changed_userid; ?>]
					</td>
				</tr>
			<?php
		}
		?>
		</table>
		<?php

	}


}//end class

?>