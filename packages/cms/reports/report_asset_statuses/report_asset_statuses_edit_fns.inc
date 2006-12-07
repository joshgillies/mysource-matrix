<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: report_asset_statuses_edit_fns.inc,v 1.6 2006/12/07 00:04:25 emcdonald Exp $
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
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage cms
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
	* @param object	&$job		a reference to the array of information about
	* @param array	&$step_data	a reference to the array of information about
	*							the current step of the HIPO job that is running this report
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$owner =& $am->getAsset($job->_running_vars['assetid'], 'report_asset_statuses');
		$where = Array();
		$tables = Array('ast a');

		$root_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, '', FALSE, 'root');
		if (!empty($root_link)) {
			// need to restrict results according to link tree position too
			$parent_treeid = $db->getOne('SELECT t.treeid FROM sq_ast_lnk_tree t WHERE linkid IN (SELECT linkid FROM sq_ast_lnk WHERE minorid = '.$root_link['minorid'].')');
			assert_valid_db_result($parent_treeid);
			$where[] = 'SUBSTR(t.treeid, 1, '.strlen($parent_treeid).') = '.$db->quote($parent_treeid);
			$tables[] = 'ast_lnk l';
			$tables[] = 'ast_lnk_tree t';
			$where[] = 'l.linkid = t.linkid';
			$where[] = 'a.assetid = l.minorid';
		}

		$search_status = str_replace('|', ', ', $owner->attr('search_status'));
		if ($search_status != SQ_SC_STATUS_ALL) {
			$where[] = 'a.status in ('.$search_status.')';
		}

		$period_length = $owner->attr('status_change_period');
		if ($period_length) {
			$operator = $owner->attr('status_change_comparator_more_than') ? '<' : '>';
			$timestamp = time() - $period_length;
			$where[] = 'a.status_changed '.$operator.' '.$db->quote(ts_iso8601($timestamp));
		}

		foreach ($tables as $index => $table) {
			$tables[$index] = 'sq_'.$table;
		}
		$sql = 'SELECT a.assetid, a.type_code, a.name, a.status, a.status_changed, a.status_changed_userid FROM '.implode(', ', $tables).' WHERE ('.implode(') AND (', $where).')';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		if (empty($result)) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			$job->_addError('No assets were found matching your criteria', TRUE);
			return FALSE;
		}
		// find the users' names
		$userids = Array();
		foreach ($result as $r) {
			if (!empty($r['status_changed_userid']) && (FALSE === strpos($r['status_changed_userid'], ':'))) {
				$userids[] = $r['status_changed_userid'];
			}
		}
		$names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($userids, Array(), FALSE, 'name');
		foreach ($result as $i => $r) {
			if (isset($names[$r['status_changed_userid']])) {
				$result[$i]['user_name'] = array_get_index($names, $r['status_changed_userid'], 'Unknown');
			} else if (!empty($r['status_changed_userid'])) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($r['status_changed_userid']);
				$result[$i]['user_name'] = $asset->name;
			} else {
				$result[$i]['user_name'] = 'Unknown';
			}
		}

		// generate XML contents
		$contents = '<asset_list>'."\n";
		foreach ($result as $values) {
			$contents .= '<asset assetid="'.$values['assetid'].'" name="'.htmlSpecialChars($values['name']).'" type_code="'.$values['type_code'].'">'."\n";
			$contents .= '	<status value="'.$values['status'].'" changed="'.$values['status_changed'].'" changed_userid="'.$values['status_changed_userid'].'" changed_user_name="'.$values['user_name'].'"/>'."\n";
			$contents .= '</asset>'."\n";
		}
		$contents .= '</asset_list>';



		$report_dir = $owner->data_path;
		$xml_file_name = $report_dir.'/report.xml';

		if (!is_dir($report_dir)) {
			if (!create_directory($report_dir)) {
				trigger_localised_error('CORE0198', E_USER_WARNING);
				return FALSE;
			}
		}

		// file manipulation section
		$file_handler = fopen($xml_file_name, 'w');
		if ($file_handler === FALSE) {
			trigger_localised_error('CORE0017', E_USER_WARNING);
			return FALSE;
		}
		fwrite($file_handler, $contents);
		fclose($file_handler);

		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		unset($owner->_tmp['getAssetMapLinks']);
		unset($owner->_tmp['getLinks']);
		return TRUE;

	}//end generateReport()


	/**
	* Paints the field for choosing the period of last status change
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintLastStatusChange(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$comparator_attr =& $asset->getAttribute('status_change_comparator_more_than');
		$comparator_attr->setEditParam('true_text', 'more');
		$comparator_attr->setEditParam('false_text', 'less');
		$comparator_attr->paint($prefix.'_comparator', !$write_access);
		echo ' than ';
		$comparator_attr =& $asset->getAttribute('status_change_period');
		$comparator_attr->paint($prefix.'_period', !$write_access);
		echo ' ago';
		return TRUE;

	}//end paintLastStatusChange()


	/**
	* Process that last status change time
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processLastStatusChange(&$asset, &$o, $prefix)
	{
		$comparator_attr =& $asset->getAttribute('status_change_comparator_more_than');
		$comparator_attr->process($prefix.'_comparator');
		$asset->setAttrValue('status_change_comparator_more_than', $comparator_attr->value);
		$period_attr =& $asset->getAttribute('status_change_period');
		$period_attr->process($prefix.'_period');
		$asset->setAttrValue('status_change_period', $period_attr->value);
		return TRUE;

	}//end processLastStatusChange()


	/**
	* Paint the interface for viewing a report
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
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

		$input =& new XML_Tree($report_path);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_localised_error('CORE0092', E_USER_WARNING, $report_path, $root->getMessage(), $root->getUserInfo());
			return;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header" style="width: 10ex"><?php echo translate('cms_reports_asset_statuses_asset_icon'); ?></td>
				<th class="sq-backend-table-header" style="width: 10ex"><?php echo translate('assetid'); ?></td>
				<th class="sq-backend-table-header"><?php echo translate('name'); ?></td>
				<th class="sq-backend-table-header"><?php echo translate('status'); ?></td>
				<th class="sq-backend-table-header"><?php echo translate('cms_reports_asset_statuses_last_status_change'); ?></td>
				<th class="sq-backend-table-header"><?php echo translate('cms_reports_asset_statuses_last_status_changer'); ?></td>
			</tr>
		<?php
		$cmp_func = create_function('$a, $b', 'return $a->attributes[\'type_code\'] > $b->attributes[\'type_code\'];');
		usort($root->children, $cmp_func);
		foreach ($root->children as $asset_node) {
			$assetid = $asset_node->attributes['assetid'];
			$name = $asset_node->attributes['name'];
			$type_code = $asset_node->attributes['type_code'];
			$type_icon = $am->getAssetIconURL($type_code);

			$status = $asset_node->children[0]->attributes['value'];
			$status_changed = $asset_node->children[0]->attributes['changed'];
			$status_changed_userid = $asset_node->children[0]->attributes['changed_userid'];
			$status_changed_user_name = $asset_node->children[0]->attributes['changed_user_name'];

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
						<?php echo get_asset_status_icon($status); ?><?php echo get_status_description($status); ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $status_changed; ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $status_changed_user_name; ?> [<?php echo $status_changed_userid; ?>]
					</td>
				</tr>
			<?php
		}//end foreach
		?>
		</table>
		<?php

	}//end paintReport()


	/**
	* Paint the interface for asset status selection
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintSearchStatuses(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$current_statuses = explode('|',$asset->attr('search_status'));

		if ($write_access) {
			$statuses = get_constant_values('SQ_STATUS');
			$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}
			combo_box($prefix.'_statuses', $options, TRUE, $current_statuses);
		} else {
			if (empty($current_statuses)) {
				echo translate('cms_listing_no_status_selected');
			} else {
				?><ul style="margin: 1px 15px;"><?php
				foreach ($current_statuses as $status) {
					echo '<li>';
					echo get_asset_status_icon($status);
					echo get_status_description($status);
					echo '</li>';
				}
				?></ul><?php
			}
		}

	}


	/**
	* Process asset selections
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSearchStatuses(&$asset, &$o, $prefix)
	{
		$new_statuses = Array();
		if (isset($_REQUEST[$prefix.'_statuses'])) {
			$new_statuses = implode('|', $_REQUEST[$prefix.'_statuses']);
			$asset->setAttrValue('search_status', $new_statuses);
		}

	}


}//end class

?>
