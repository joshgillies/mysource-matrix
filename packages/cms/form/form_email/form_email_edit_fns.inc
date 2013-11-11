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
* $Id: form_email_edit_fns.inc,v 1.115 2013/08/06 04:51:19 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../form/form_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

/**
* Form_Email_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.115 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Email_Edit_Fns extends Form_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Form_Email_Edit_Fns()
	{
		$this->Form_Edit_Fns();

	}//end constructor


//--        SUBMISSIONS        --//


	/**
	* Paints the date range selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionDateRange(&$asset, &$o, $prefix)
	{
		$parameters = Array(
						'min'			=> '2003-01-01 00:00:00',
						'max'			=> ts_iso8601(time()),
						'allow_circa'	=> '0',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array(),
						'style'			=> Array(
											'y'	=> 's',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_fromvalue']['m'],$_POST[$prefix.'_fromvalue']['d'],$_POST[$prefix.'_fromvalue']['y'])) : date('Y-m-d',time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_tovalue']['m'],$_POST[$prefix.'_tovalue']['d'],$_POST[$prefix.'_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from = new Datetime_Field($prefix.'_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_to', $to_value, $parameters);

		$from->printField();
		echo ' '.translate('to').' ';
		$to->printField();

		return TRUE;

	}//end paintSubmissionDateRange()


	/**
	* Paints the submission count interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionCount(&$asset, &$o, $prefix)
	{
		$num_per_page = array_get_index($_REQUEST, $prefix.'_per_page', 50);
		if ($num_per_page < 1) $num_per_page = 50;

		text_box($prefix.'_per_page', $num_per_page, 3);

	}//end paintSubmissionCount()


	/**
	* Paints the submission list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionList(&$asset, &$o, $prefix)
	{	
		$write_access = $asset->writeAccess('links');
		$expand_list = isset($_POST[$prefix.'_expand']) ? $_POST[$prefix.'_expand'] : Array();
		$num_subs = 0;

		// grab the number of assets per page, defaulting to 50 at a time
		$num_per_page = array_get_index($_REQUEST, $prefix.'_per_page', 50);
		if ($num_per_page < 1) $num_per_page = 50;

		$from_value = isset($_POST[$prefix.'_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_fromvalue']['m'],$_POST[$prefix.'_fromvalue']['d'],$_POST[$prefix.'_fromvalue']['y'])) : date('Y-m-d',time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_tovalue']['m'],$_POST[$prefix.'_tovalue']['d'],$_POST[$prefix.'_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from_value = iso8601_date_component($from_value).' 00:00:00';
		$to_value = iso8601_date_component($to_value).' 23:59:59';

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$sql = 'SELECT
					a.assetid, a.created_userid, v.custom_val AS submitted
				FROM
					('.SQ_TABLE_RUNNING_PREFIX.'ast a
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON l.minorid = a.assetid
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val v ON v.assetid = a.assetid
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr n ON n.attrid = v.attrid';

		$count_sql = 'SELECT
						count(*)
					  FROM
						('.SQ_TABLE_RUNNING_PREFIX.'ast a
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l ON l.minorid = a.assetid
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val v ON v.assetid = a.assetid
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr n ON n.attrid = v.attrid';

		$sub_folder = $asset->getSubmissionsFolder();

		$db = MatrixDAL::getDb();
		$where = 'l.majorid IN (:assetid, :subfolder_assetid)
					AND i.inhd_type_code = :inhd_type_code
					AND n.name = :name AND v.contextid = :context';
		if (MatrixDAL::getDbType() == 'oci') {
			$where .= ' AND TO_CHAR(DBMS_LOB.SUBSTR(v.custom_val, 30, 1)) > :from_date AND TO_CHAR(DBMS_LOB.SUBSTR(v.custom_val, 30, 1)) < :to_date';
			$order  = ' ORDER BY TO_CHAR(DBMS_LOB.SUBSTR(v.custom_val, 30, 1)) DESC';
		} else {
			$where .= ' AND v.custom_val BETWEEN :from_date AND :to_date';
			$order  = ' ORDER BY v.custom_val DESC';
		}//end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'i');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'n');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'v');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

		// grab the number of assets

		try {
			$query = MatrixDAL::preparePdoQuery($count_sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid',			$asset->id);
			MatrixDAL::bindValueToPdo($query, 'subfolder_assetid',	$sub_folder->id);
			MatrixDAL::bindValueToPdo($query, 'inhd_type_code',		'form_submission');
			MatrixDAL::bindValueToPdo($query, 'name',   			'submitted');
			MatrixDAL::bindValueToPdo($query, 'context',   			$GLOBALS['SQ_SYSTEM']->getContextId());
			MatrixDAL::bindValueToPdo($query, 'from_date',			$from_value);
			MatrixDAL::bindValueToPdo($query, 'to_date',			$to_value);
			$asset_count = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get the number of assets due to database error: '.$e->getMessage());
		}

		$num_pages = ceil($asset_count / $num_per_page);
		$page_num = array_get_index($_POST, $prefix.'_submissions_page', 1);
		$page_num = max($page_num, 1);
		$page_num = min($page_num, $num_pages);

		$submission_offset = ($page_num - 1) * $num_per_page;

		$sql = $sql.$where.$order;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), $num_per_page, $submission_offset);

		if ($asset_count == 0) {
			?><p><i><?php echo translate('cms_form_no_submissions_during_period') ?></i></p><?php
		} else {
			if ($num_pages == 1) {
				$page_tag = translate('page_number', $page_num, $num_pages);
			} else {
				$page_tag = translate('page_number_with_pager', $page_num, $num_pages, $prefix.'_submissions_page');
			}
			$asset_count_tag = translate('item_range', ($page_num - 1) * $num_per_page + 1, min($asset_count, $page_num * $num_per_page), $asset_count, strtolower(translate('submissions')));

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('pagination_go_to_first').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_submissions_page\', 1)">'.$links['first'].'</a>';
				$links['previous'] = '<a title="'.translate('pagination_go_to_previous').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_submissions_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';
			} else {
				$links['first'] = '<span title="'.translate('pagination_cannot_go_further_back').'." style="color: #333">'.$links['first'].'</span>';
				$links['previous'] = '<span title="'.translate('pagination_cannot_go_further_back').'." style="color: #333">'.$links['previous'].'</span>';
			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('pagination_go_to_last').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_submissions_page\', '.$num_pages.')">'.$links['last'].'</a>';
				$links['next'] = '<a title="'.translate('pagination_go_to_next').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_submissions_page\', '.($page_num + 1).')">'.$links['next'].'</a>';
			} else {
				$links['last'] = '<span title="'.translate('pagination_cannot_go_further_forward').'." style="color: #333">'.$links['last'].'</span>';
				$links['next'] = '<span title="'.translate('pagination_cannot_go_further_forward').'." style="color: #333">'.$links['next'].'</span>';
			}

			hidden_field($prefix.'_submissions_page', $page_num); ?>

			<style type="text/css">
				td.sq-form-sub {
					padding-top: 0px;
				}
				.sq-form-sub table.sq-backend-table {
					border-top: 0px;
					margin: 0 8px 8px 8px;
				}
				.sq-form-sub table.sq-backend-table th {
					font-weight: normal;
					text-decoration: italic;
				}
				table.sq-backend-table tr td.sq-form-sub-header {
					background-color: #666;
					color: #fff;
					font-size: 11px;
				}
			</style>
			<script type="text/javascript"><!--
				function sq_set_check_all(form, prefix, value) {
					set_els = form.elements[prefix];

					if (set_els.length) {
						for (k = 0; k < set_els.length; k++) {
							set_els[k].checked = value;
						}
					} else {
						set_els.checked = value;
					}
				}

				function sq_test_check_all(form, prefix, set_el) {
					test_els = form.elements[prefix];
					set_el = form.elements[set_el];

					if (test_els.length) {
						test = true;
						for (k = 0; k < test_els.length; k++) {
							if (!test_els[k].checked) {
								test = false;
								break;
							}
						}
					} else {
						test = test_els.checked;
					}

					set_el.checked = test;
				}
			//--></script>
			<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header-header" colspan="2"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></td>
				<td class="sq-backend-table-header-header" colspan="<?php echo ($write_access === TRUE) ? 4 : 3 ?>" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></td>
			</tr>
			<!-- <tr>
			<td class="sq-form-sub-header" colspan="2"><b><?php
				hidden_field($prefix.'_submission_offset', $submission_offset);
				if ($submission_offset > 0) {
					?><a title="<?php echo translate('cms_form_submissions_go_to_first') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('cms_form_cannot_go_further_back') ?>" style="color: #333"><?php
				}
				?>
				&lt;&lt;
				<?php
				if ($submission_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if ($submission_offset > 0) {
					?><a title="<?php echo translate('cms_form_submissions_go_to_previous') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo $submission_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('cms_form_cannot_go_further_back') ?>" style="color: #333"><?php
				}
				?>
				&lt;
				<?php
				if ($submission_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?>
				&nbsp; &nbsp; <?php echo translate('page_number', round(($submission_offset + $num_per_page) / $num_per_page), ceil($asset_count / $num_per_page)); ?> &nbsp; &nbsp;
				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?><a title="<?php echo translate('cms_form_submissions_go_to_next') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo $submission_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('cms_form_cannot_go_further_forward') ?>" style="color: #333"><?php
				}
				?>
				&gt;
				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?><a title="<?php echo translate('cms_form_submissions_go_to_last') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('cms_form_cannot_go_further_forward') ?>" style="color: #333"><?php
				}
				?>
				&gt;&gt;
				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?></a><?php
				} else {
					?></span><?php
				}
				?></b>
			</td>
			<td class="sq-form-sub-header" colspan="4" align="right"><?php echo translate('cms_form_submissions_during_period', $submission_offset + 1, min($asset_count, $submission_offset + $num_per_page), $asset_count) ?>
			</td>
			</tr> -->
			<tr>
				<th>
					<?php echo translate('asset_id') ?>
				</th>
				<th>
					<?php echo translate('submission_time') ?>
				</th>
				<th>
					<?php echo translate('user') ?>
				</th>
				<th>
					<input type="checkbox" name="<?php echo $prefix ?>_expand_all" value="" onclick="sq_set_check_all(this.form, '<?php echo $prefix ?>_expand[]', this.checked);"> <?php echo translate('expand_question') ?>
				</th>
			<?php
			if ($write_access) {
				?>
				<th>
					<input type="checkbox" name="<?php echo $prefix ?>_delete_all" value="" onclick="sq_set_check_all(this.form, '<?php echo $prefix ?>_delete[]', this.checked);"> <?php echo translate('delete_question') ?>
				</th>
				<?php
			}
			?>
				<th>&nbsp;</th>
			</tr>
			<?php

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid',			$asset->id);
				MatrixDAL::bindValueToPdo($query, 'subfolder_assetid',	$sub_folder->id);
				MatrixDAL::bindValueToPdo($query, 'inhd_type_code',		'form_submission');
				MatrixDAL::bindValueToPdo($query, 'name',				'submitted');
				MatrixDAL::bindValueToPdo($query, 'context',   			$GLOBALS['SQ_SYSTEM']->getContextId());
				MatrixDAL::bindValueToPdo($query, 'from_date',			$from_value);
				MatrixDAL::bindValueToPdo($query, 'to_date',			$to_value);
				$result = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get submission list due to database error: '.$e->getMessage());
			}

			foreach ($result as $asset_info) {
				$assetid = $asset_info['assetid'];

				// time is good, load me up
				$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$sub_user = $asset_info['created_userid'];
				$sub_time = date('d M Y H:i:s', strtotime($asset_info['submitted']));
				$sub_expanded = array_search($assetid, $expand_list) !== FALSE;
				$num_subs++;

				if ($num_subs > $num_per_page) break;
				?>
				<tr<?php
					echo ($sub_expanded) ? ' class="alt"' : '';
				?>>
					<td>
						<?php echo $assetid ?>
					</td>
					<td>
						<?php echo $sub_time ?>
					</td>
					<td>
						<?php
						$valid_user = FALSE;
						if (!empty($sub_user)) {
							$sub_user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_user);
							if (!empty($sub_user_asset)) {
								echo get_asset_tag_line($sub_user);
								$valid_user = TRUE;
							}
						}
						if (!$valid_user) {
							?><i><?php echo translate('cms_form_no_logged_in_user'); ?></i><?php
						}
						?>
					</td>
					<td>
						<input type="checkbox" name="<?php echo $prefix ?>_expand[]" value="<?php echo $assetid ?>" <?php if ($sub_expanded) echo ' checked="checked"'; ?> onclick="sq_test_check_all(this.form, '<?php echo $prefix ?>_expand[]', '<?php echo $prefix ?>_expand_all');">
					</td>
				<?php
				if ($write_access) {
					?>
					<td>
						<input type="checkbox" name="<?php echo $prefix ?>_delete[]" value="<?php echo $assetid ?>" onclick="sq_test_check_all(this.form, '<?php echo $prefix ?>_delete[]', '<?php echo $prefix ?>_delete_all');">
					</td>
					<?php
				}
				?>
					<td>[ <a href="<?php echo $sub_asset->getBackendHref(); ?>"><?php echo translate('edit') ?></a> ]</td>
				</tr>
				<?php
				// display only if expanded
				if ($sub_expanded) {
					?>
					<tr>
						<td colspan="6" class="sq-form-sub">
							<table class="sq-backend-table">
								<tr>
									<?php
									$i = 0;
									$elements = $sub_asset->getSummary();
									if (count($elements) % 2 != 0) {
										$elements[] = Array('name' => '', 'answer' => '');
									}
									foreach ($elements as $element) {
										$i++;
										?>
										<th width="10%"><?php echo $element['name']; if (!empty($element['name'])) echo ':'; ?></th>
										<td width="40%"><?php echo $element['answer']; ?></td>
										<?php
										if (($i < count($elements)) && ($i % 2 == 0)) {
											?>
								</tr>
								<tr>
											<?php
										}
									}//end foreach elemnent
									?>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}//end if expanded
			}//end while submission
			?>
			</table>
			<script type="text/javascript"><!--
			sq_test_check_all(document.main_form, '<?php echo $prefix ?>_expand[]', '<?php echo $prefix ?>_expand_all');
			//--></script>
		<?php
		}//end else


		return TRUE;

	}//end paintSubmissionList()


	/**
	* Processes the submissions block (only processes deletion of submissions)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissionList(&$asset, &$o, $prefix)
	{
		$delete_list = isset($_POST[$prefix.'_delete']) ? $_POST[$prefix.'_delete'] : Array();
		if (empty($delete_list)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// acquire the links lock on the form
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'links')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		foreach ($delete_list as $delete_assetid) {
			// is the assetid a valid one?
			if (!assert_valid_assetid($delete_assetid, '', FALSE, FALSE)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			// acquire the links lock on the submission to delete
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($delete_assetid, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			$submissions_folder = $asset->getSubmissionsFolder();
			$submission_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($submissions_folder->id, $delete_assetid, SQ_LINK_TYPE_3);
			if (!$submission_link) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			} else {
				if (!$submissions_folder->deleteLink($submission_link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			// release the links lock
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($delete_assetid, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach

		// now release the links lock on the form itself
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processSubmissionList()


	/**
	* Paints the download date range selector for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadDateRange(&$asset, &$o, $prefix)
	{
		$parameters = Array(
						'min'			=> '2003-01-01 00:00:00',
						'max'			=> ts_iso8601(time()),
						'allow_circa'	=> '0',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array(),
						'style'			=> Array(
											'y'	=> 's',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : date('Y-m-d', time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from = new Datetime_Field($prefix.'_export_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_export_to', $to_value, $parameters);

		$from->printField();
		echo ' '.translate('to').' ';
		$to->printField();

	}//end paintDownloadDateRange()


	/**
	* Paints the download format selector for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadFormat(&$asset, &$o, $prefix)
	{
		$options = Array(
					''			=> '------- '.translate('please_select').' -------',
					'csv'		=> translate('cms_form_download_csv'),
					'csv_nhr'	=> translate('cms_form_download_csv_nhr'),
					'xml'		=> translate('cms_form_download_xml'),
				   );

		echo combo_box($prefix.'_export_format', $options, FALSE, '');
		echo hidden_field($prefix.'_export_switch', '0');

		$delimiter_text = isset($_POST[$prefix.'_delimiter']) ? $_POST[$prefix.'_delimiter'] : ',';
		echo ' &nbsp; Delimiter &nbsp;';
		echo text_box($prefix.'_delimiter',$delimiter_text,1,1,TRUE);

	}//end paintDownloadFormat()


	/**
	* Paints the download button for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadButton(&$asset, &$o, $prefix)
	{
		// explanation for onClick for the Download button, there is some trickery involved here >:)
		// - set export switch to 1 = "ON"
		// - submit form (automatically sets SQ_FORM_SUBMITTED) - we're still here though, as the
		//   browser will be sent through the CSV/XML file and it will be offered to save - not display
		//   (DEPENDS on browser - CSV should be offered to save, XML could be intercepted by browser!)
		// - unset SQ_FORM_SUBMITTED so the form can still be submitted via Commit
		// - set export switch to 0 = "OFF" so Commit doesn't try to save a file either
		// - return true for the event as per JS standard

		echo normal_button($prefix.'_export', translate('download_file'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');

	}//end paintDownloadButton()


	/**
	* Processes the Download Button press
	*
	* This function will pass off the work to processExportCSV() or processExportXML(), as appropriate.
	* If we aren't downloading a file, this function will silently return a success.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDownloadButton(&$asset, &$o, $prefix)
	{
		$export_button_pressed = array_get_index($_POST, $prefix.'_export_switch');
		if (is_null($export_button_pressed)) return TRUE;

		$export_type = array_get_index($_POST, $prefix.'_export_format', '');
		$export_delimiter = array_get_index($_POST, $prefix.'_delimiter', '');

		switch ($export_type) {
			case 'csv':
				$this->processExportCSV($asset, $o, $prefix, TRUE, $export_delimiter);
			break;

			case 'csv_nhr':
				$this->processExportCSV($asset, $o, $prefix, FALSE, $export_delimiter);
			break;

			case 'xml':
				$this->processExportXML($asset, $o, $prefix);
			break;
		}

		return TRUE;

	}//end processDownloadButton()


	/**
	* Exports chosen submissions to a comma separated value (CSV) file
	*
	* Will only perform export for the date range which was
	* selected in the Export block.
	*
	* @param object		&$asset		the asset whose interface we are painting
	* @param object		&$o			the outputter class
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$header_row	whether the header row is filled in
	* @param string		$delimiter	delimiter char
	*
	* @return boolean
	* @access public
	*/
	function processExportCSV(&$asset, &$o, $prefix, $header_row=TRUE, $delimiter=',')
	{
		// get the from and to values
		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from_value = explode('-',iso8601_date_component($from_value));
		$to_value = explode('-',iso8601_date_component($to_value));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$csv = $this->createCSVSubmissionLogs($asset, $from_ts, $to_ts);
		$csv->setFilename('submission_log.csv');
		$csv->setDeliminator($delimiter);

		// by default, headers are set in createCSVSubmissionLogs(). If we do not need it, set it to FALSE
		if (!$header_row) $csv->setFieldHeaders($header_row); // setFieldHeaders() accept pass-by-reference parameter so we just can not use FALSE literal value alone

		// export with keyed fields
		$csv->export(TRUE);
		exit(0);

		return TRUE;

	}//end processExportCSV()


	/**
	 * Create a CSV submission log of all the Form submissions in a period of time.
	 *
	 * @param Form $form		The Form that submissions were submitted to
	 * @param int $from_time	The Unix timestamp of the from time
	 * @param int $to_time		The Unix timestamp of the to time
	 */
	public function createCSVSubmissionLogs($form, $from_time, $to_time)
	{
		$logs = Array();
		$headers = Array('__time__' => translate('submission_time'), '__user__' => translate('cms_form_user_submitting'), '__ipaddress__' => translate('cms_form_ipaddress_submitting'));
		$sub_folder = $form->getSubmissionsFolder();
		$assetids = Array($form->id, $sub_folder->id);
		$from_time = ts_iso8601($from_time);
		$to_time = ts_iso8601($to_time);

		try {
			$bind_vars = Array(
							'assetids'	=> $assetids,
							'from_time'	=> $from_time,
							'to_time'	=> $to_time,
							'name'		=> 'submitted',
							'context'	=> $GLOBALS['SQ_SYSTEM']->getContextId(),
						 );
			$result = $this->_getSubmissionLog($bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to get submission log due to database error: ".$e->getMessage());
		}

		foreach ($result as $asset_info) {
			$assetid = $asset_info['assetid'];
			$type_code = $asset_info['type_code'];

			$create_time = strtotime($asset_info['submitted']);

			$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (!$sub_asset->attr('complete')) continue;

			// get the submission ip
			$ipaddress = $sub_asset->getIP();

			// get the summary
			$summary = $sub_asset->getSummary();
			$logs[$assetid] = Array(
								'__time__'		=> date('Y-m-d H:i:s', $create_time),
								'__user__'		=> $asset_info['created_userid'],
								'__ipaddress__'	=> $ipaddress,
							  );
			if (empty($summary)) $summary = Array();
			foreach ($summary as $id => $element) {
				if (!isset($headers[$id])) {
					$headers[$id] = $element['name'];
				}

				$logs[$assetid][$id] = html_entity_decode($element['answer'], ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET) ;
			}
			$logs[$assetid]['__assetid__'] = $assetid;

			// Finally, add the header row for assetid in
			if (!isset($headers['__assetid__'])) {
				$headers['__assetid__'] = translate('assetid');
			}//end if
		}//end foreach

		require_once SQ_FUDGE_PATH.'/csv/csv.inc';
		$csv = new CSV();
		$csv->setFieldHeaders($headers);
		$csv->setValues($logs);

		return $csv;

	}//end createCSVSubmissionLogs()


	/**
	* Exports chosen submissions to an XML format
	*
	* XML format is the same format used to store the submission logs, with an overarching
	* "submissions" root tag around it. Will only perform export for the date range which was
	* selected in the Export block.
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExportXML(&$asset, &$o, $prefix)
	{
		header('Content-Type: application/xml');

		// defining the file as attachment forces most browsers to offer it for download
		header('Content-Disposition: attachment; filename="submission_log.xml";');

		//This is known that Internet Explorer has problems with SSL connection (https).
		//http://support.microsoft.com/default.aspx?scid=kb;en-us;812935
		//http://support.microsoft.com/default.aspx?scid=kb;en-us;316431
		// do not send no cache header in https for IE
		$url_info = parse_url(current_url());
		$protocol = (isset($url_info['scheme'])) ? $url_info['scheme'] : NULL;
		$using_ie6_8 = isset ($_SERVER['HTTP_USER_AGENT']) ? preg_match('/(MSIE\s[0-8]+\.)/', $_SERVER['HTTP_USER_AGENT']) : FALSE;
		if (!is_null($protocol) && $protocol == 'https' && $using_ie6_8) {
			header('Cache-Control: private, max-age=0, must-revalidate');
			header('Pragma: private');
			header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
		}

		// get the from and to values
		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from_value = explode('-', iso8601_date_component($from_value));
		$to_value = explode('-', iso8601_date_component($to_value));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0, 0, 0, $from_value[1], $from_value[2], $from_value[0]);
		$to_ts   = mktime(23, 59, 59, $to_value[1], $to_value[2], $to_value[0]);
		$from_time = ts_iso8601($from_ts);
		$to_time = ts_iso8601($to_ts);
		$sub_folder = $asset->getSubmissionsFolder();
		$assetids = Array($asset->id, $sub_folder->id);
		
		try {
			$bind_vars = Array(
							'assetids'	=> $assetids,
							'from_time'	=> $from_time,
							'to_time'	=> $to_time,
							'name'		=> 'submitted',
							'context'	=> $GLOBALS['SQ_SYSTEM']->getContextId(),
						 );
			$result = $this->_getSubmissionLog($bind_vars);
		} catch (Exception $e) {
			throw new Exception("Unable to get submission log due to database error: ".$e->getMessage());
		}

		echo "<submissions>\n";
		foreach ($result as $asset_info) {
			$assetid = $asset_info['assetid'];
			$type_code = $asset_info['type_code'];

			$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
			if (!$sub_asset->attr('complete')) continue;

			if (trim($sub_asset->attr('xml')) == '') continue;
			echo "\t".$sub_asset->attr('xml')."\n";
		}
		echo '</submissions>';
		exit(0);

		return TRUE;

	}//end processExportXML()
	
	
	/**
	* Paints the interface for the existing "Send Submissions Email" jobs
	*
	* @param Form_Email			$asset		The form whose interface we are painting
	* @param Backend_Outputter	$o			the outputter class
	* @param string				$prefix		prefix for form elements
	*
	* @return boolean
	*/
	public function paintSendSubmissionsEmail(Form_Email $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		
		$job_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'cron_job_send_submission_log', TRUE, 'minor');		

		if (!empty($job_links)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('name'); ?></td>
					<td class="sq-backend-table-header"><?php echo translate('when'); ?></td>					
					<td align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('delete'); ?> ?</td>
					<td align="center" class="sq-backend-table-header" style="text-align: center;"></td>
				</tr>
				<?php
				for ($k = 0; $k < count($job_links); $k++) {
					$job = $GLOBALS['SQ_SYSTEM']->am->getAsset($job_links[$k]['majorid'], 'cron_job_send_submission_log');
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $job->displayName(); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $job->readableWhen(); ?>
						</td>
						<td align="center" class="sq-backend-table-cell">
						<?php
							if ($write_access && $job->canDelete()) {
								check_box($prefix.'_remove_job[]', $job->id);
							} else {
								check_box($prefix.'_remove_job[]', $job->id, FALSE, '', 'disabled');
							}
						?>
						</td>						
						<td>[ <a href="<?php echo $job->getBackendHref(); ?>"><?php echo translate('edit') ?></a> ]</td>
					</tr>
					<?php
				}//end for
				?>
			</table>
			<?php

		} else {
			echo "<i>".translate('cron_job_send_submission_log_no_jobs')."</i><br/><br/>";
		}		
		
		// Add new job checkbox
		if ($write_access) {			
			label(translate('cron_job_send_submission_log_add_new_job'), $prefix.'_new_job');
			echo '&nbsp;';
			text_box($prefix.'_new_job_name', '', 25);
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_send_submission_log');
			$cron_job = new Cron_Job_Send_Submission_Log();
			$edit_fns = $cron_job->getEditFns();
			$null_asset = NULL;
			$edit_fns->paintScheduleInterface($null_asset, $o, $prefix);
		}	
	
		return $write_access;

	}//end paintSendSubmissionsEmail()
	
	
	/**
	* Processes the interface for the existing "Send Submissions Email" jobs
	*
	* @param Form_Email			$asset		The form whose interface we are painting
	* @param Backend_Outputter	$o			the outputter class
	* @param string				$prefix		prefix for form elements
	*
	* @return boolean
	*/
	public function processSendSubmissionsEmail(Form_Email $asset, Backend_Outputter $o, $prefix)
	{	
		$write_access = $asset->writeAccess('attributes');		
		
		if ($write_access) {
			
			$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_send_submission_log');
			
			// Delete selected jobs
			if (isset($_POST[$prefix.'_remove_job'])) {
				$jobs = Cron_Job_Send_Submission_Log::getActiveJobs($asset);
				if (!empty($jobs)) {

					if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
						for ($i = 0; $i < count($jobs); $i++) {
							if (in_array($jobs[$i]->id, $_POST[$prefix.'_remove_job'])) {
								$cron_mgr->removeJob($jobs[$i]); 
							}// end if
						}// end for
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');
					} else {
						trigger_localised_error('SYS0232', E_USER_NOTICE);

					}// end if

				}// end if			
			}//end if
				
			// Create new cron job
			$new_job_name = isset($_POST[$prefix.'_new_job_name']) ? trim($_POST[$prefix.'_new_job_name']) : '';
			if (!empty($new_job_name)) {				
				
				$default_time = 'TI='.time().',7,86400';
				$new_job = new Cron_Job_Send_Submission_Log();
				$new_job->setAttrValue('display_name', $new_job_name);
				$new_job->setAttrValue('type', 'repeating');
				$new_job->setAttrValue('long', TRUE);
				$edit_fns = $new_job->getEditFns();
				if (!$edit_fns->processScheduleInterface($new_job, $o, $prefix)) {
					return FALSE;
				}
				
				if ($new_job->setAssetToUpdate($asset)) {
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
					if ($cron_mgr->addJob($new_job, $GLOBALS['SQ_SYSTEM']->user)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');						
					} else {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);		
			}//end if			
		}//end if
		
		return $write_access;

	}//end processSendSubmissionsEmail()
	
	
	/**
	* Get the submission log from the database
	*
	* @param array	$bind_vars	the bind vars to process
	*
	* @return array
	* @access private
	*/
	function _getSubmissionLog($bind_vars)
	{
		$result = Array();

		// No assetids passed in ... no results for you!
		if (isset($bind_vars['assetids']) && empty($bind_vars['assetids'])) {
			return $result;
		}//end if

		try {
			if (MatrixDAL::getDbType() === 'oci') {
				$sql = "SELECT a.assetid, a.type_code, v.custom_val AS submitted, a.created_userid
						 FROM sq_ast a
						 INNER JOIN sq_ast_typ_inhd t ON a.type_code=t.type_code
						 INNER JOIN sq_ast_lnk l ON l.minorid=a.assetid
						 INNER JOIN sq_ast_attr_val v ON v.assetid=a.assetid
						 INNER JOIN sq_ast_attr n ON n.attrid=v.attrid";
				$where = " WHERE";
				$assetids = array_get_index($bind_vars, 'assetids', Array());
				if (!empty($assetids)) {
					foreach ($assetids as $index => $assetid) {
						$assetids[$index] = MatrixDAL::quote($assetid);
					}//end foreach
					$where .= " l.majorid IN (".implode(',', $assetids).") AND";
				}//end if
				$where .= " t.inhd_type_code='form_submission'
							AND n.name=:name AND v.contextid=:context
							AND TO_CHAR(DBMS_LOB.SUBSTR(v.custom_val, 30, 1)) > :from_time
							AND TO_CHAR(DBMS_LOB.SUBSTR(v.custom_val, 30, 1)) < :to_time";
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'name', array_get_index($bind_vars, 'name', ''));
				MatrixDAL::bindValueToPdo($query, 'context', array_get_index($bind_vars, 'context', ''));
				MatrixDAL::bindValueToPdo($query, 'from_time', array_get_index($bind_vars, 'from_time', date('Y-m-d').' 00:00:00'));
				MatrixDAL::bindValueToPdo($query, 'to_time', array_get_index($bind_vars, 'to_time', date('Y-m-d').' 23:59:59'));
				$result = MatrixDAL::executePdoAssoc($query);
			} else {
				$result = MatrixDAL::executeAssoc('cms_package', 'getSubmissionLog', $bind_vars);
			}//end if
		} catch (Exception $e) {
			$result = Array();
			throw new Exception("Unable to get submission log due to database error: ".$e->getMessage());
		}

		return $result;

	}//end _getSubmissionLog()


	/**
	* Returns a list of name-value pairs for a submission result
	*
	* @param object	&$tree_root	the XML node that we are going to process
	*
	* @return array
	* @access private
	*/
	function &_generateSubmissionList(&$tree_root)
	{
		$children = Array();

		$kids =& $tree_root->children;

		for (reset($kids); NULL !== ($k = key($kids)); next($kids)) {
			$this_child =& $kids[$k];
			$child_id = $this_child->getAttribute('id');
			$child_name = $this_child->getAttribute('name');
			switch ($this_child->name) {
				case 'section':
				break;

				case 'text_q':
					$child_value = $this_child->content;
				break;

				case 'datetime_q':
					$child_value = date('d M Y H:i:s',	iso8601_ts($this_child->content));
					$circa = $this_child->getAttribute('circa');
					if ($circa) $child_value = 'circa '.$child_value;
				break;

				case 'select_q':
					$select_options =& $this_child->children;
					$child_value = Array();		// store child as an array

					for (reset($select_options); NULL !== ($k = key($select_options)); 	next($select_options)) {
						$this_option =& $select_options[$k];
						$child_value[] = $this_option->content;
					}
					$child_value = implode(', ', $child_value);
				break;
			}

			if ($this_child->name != 'section') {
				$children[] = Array('id' => $child_id, 'name' => $child_name, 'content' => $child_value);
			}
		}//end for

		return $children;

	}//end _generateSubmissionList()


//--        RECEIPT FIELD        --//


	/**
	* Paints the interface for selecting a field containing send to a friend entries
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintReceiptField(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$assetid = $asset->attr('receipt_field');
		if ($write_access) {
			asset_finder($prefix.'_rec_field', $assetid, Array('form_question_type_email_address' => 'I'));
		} else {
			if (!empty($assetid)) {
				echo get_asset_tag_line($assetid);
			} else {
				echo '<em>'.translate('none').'</em>';
			}
		}

		return $write_access;

	}//end paintReceiptField()


	/**
	* Processes the interface for selecting a field containing a receipt email
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processReceiptField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rec_field'])) return FALSE;
		$assetid = $_POST[$prefix.'_rec_field']['assetid'];

		// add check that this is in the same form

		// if we already have this as the not found asset, don't set it again
		if ($assetid == $asset->attr('receipt_field')) {
			return FALSE;
		}
		if (!$asset->setAttrValue('receipt_field', $assetid)) {
			return FALSE;
		}

		return TRUE;

	}//end processReceiptField()


//--        SEND TO A FRIEND        --//


	/**
	* Paints the interface for selecting a field containing send to a friend entries
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintSTAFField(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$assetid = $asset->attr('staf_field');
		if ($write_access) {
			asset_finder($prefix.'_staf_field', $assetid, Array('form_question_type_text' => 'D'));
		} else {
			if (!empty($assetid)) {
				echo get_asset_tag_line($assetid);
			} else {
				echo '<em>'.translate('none').'</em>';
			}
		}

		return $write_access;

	}//end paintSTAFField()


	/**
	* Processes the interface for selecting a field containing send to a friend entries
	*
	* @param object	&$asset	the form involved
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processSTAFField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_staf_field'])) {
			return FALSE;
		}
		$assetid = $_POST[$prefix.'_staf_field']['assetid'];

		// todo: add check that this is in the same form

		// if we already have this as the not found asset, don't set it again
		if ($assetid == $asset->attr('staf_field')) {
			return FALSE;
		}
		if (!$asset->setAttrValue('staf_field', $assetid)) {
			return FALSE;
		}

		return TRUE;

	}//end processSTAFField()


//--        SELECTIVE E-MAILS        --//


	/**
	* Paints the interface for creating a new Selective Email rule
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddSelectiveEmailRule(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		check_box($prefix.'_new_se_rule', '1', FALSE);
		echo ' New Rule';

		return TRUE;

	}//end paintAddSelectiveEmailRule()


	/**
	* Processes the interface for creating a new, blank Selective Email rule
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddSelectiveEmailRule(&$asset, &$o, $prefix)
	{
		if (!isset($_REQUEST[$prefix.'_new_se_rule'])) {
			return FALSE;
		}
		$all_se = $asset->attr('selective_emails');

		// pull out a new index for this question
		if (empty($all_se)) {
			$new_id = 0;
		} else {
			$new_id = max(array_keys($all_se)) + 1;
		}

		// fill out the defaults
		$all_se[$new_id] = Array(
							'assetid'	=> '',
							'rules'		=> Array(),
							'require'	=> 'all',
							'send'		=> 'recipient',
							'address'	=> '',
						   );

		// set the post/request variables so the later functions know to show the new rule
		$_REQUEST[$prefix.'_se_active_ruleid'] = $new_id;

		$_POST[$prefix.'_require']	 = 'all';
		$_POST[$prefix.'_send']		 = 'recipient';
		$_POST[$prefix.'_address']	 = '';

		if (!$asset->setAttrValue('selective_emails', $all_se)) {
			return FALSE;
		}

		return $asset->saveAttributes();

	}//end processAddSelectiveEmailRule()


	/**
	* paint the current rules
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
		$admin_access = $asset->adminAccess('attributes');

		$am = $GLOBALS['SQ_SYSTEM']->am;

		$ruleid = isset($_REQUEST[$prefix.'_se_active_ruleid']) ? $_REQUEST[$prefix.'_se_active_ruleid'] : -1;

		if (isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			$o->addHiddenField($prefix.'_se_active_ruleid', $_REQUEST[$prefix.'_se_active_ruleid']);
		}

		$all_se = $asset->attr('selective_emails');

		if (empty($all_se)) {
			echo translate('cms_form_no_selective_email_rules');
			return FALSE;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<?php
				if ($admin_access) {
					?><td align="center" width="30" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('edit') ?></td><?php
				}
				?><td class="sq-backend-table-header">
					<?php echo translate('rule') ?>
				</td>
				<td align="center" width="50" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('valid_question') ?></td>
				<?php
				if ($admin_access) {
					?><td align="center" width="70" class="sq-backend-table-header" style="font-weight: bold;"><?php echo translate('delete_question') ?></td>
				<?php
				}
				?>
			</tr>
		<?php
		for (reset($all_se); NULL !== ($k = key($all_se)); next($all_se)) {
			$info =& $all_se[$k];
			$rule_prefix = $prefix.'_r'.$k;
			$valid = TRUE;
			?>
			<tr<?php if ($ruleid == $k) echo ' class="alt"'; ?>>
				<?php
				if ($admin_access) {
					?>
					<td valign="top" align="center" width="30" class="sq-backend-table-cell"><a href="<?php echo $o->getCurrentLocation(); ?>&<?php echo $prefix?>_se_active_ruleid=<?php echo $k ?>"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a></td>
					<?php
				}
				?>
				<td valign="top" class="sq-backend-table-cell">
					<?php
					if (empty($info['rules'])) {
						?>
						<span><?php echo translate('cms_form_rule_no_parts') ?></span><br/>
						<?php
						$valid = FALSE;
					} else {
						$rules =& $info['rules'];

						$rule_logic = ($info['require'] == 'all') ? strtoupper(translate('and')) : strtoupper(translate('or'));
						$descriptions = Array();
						$deleted_q = FALSE;

						$i = 0;
						for (reset($rules); NULL !== ($rule_key = key($rules)); next($rules)) {
							$rule =& $rules[$rule_key];
							$type_code = 'form_question_rule_type_'.$rule['name'];
							$q_asset = $am->getAsset($rule['assetid']);

							// deleted question = invalid rule!
							if (!$q_asset) {
								$deleted_q = TRUE;
								$valid = FALSE;
								break;
							}

							$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
							$rule_asset = new $type_code();
							$descriptions[] = $rule_asset->ruleDescription($q_asset, $rule);

							$i++;
						}//end for

						if ($deleted_q) {
							echo translate('cms_form_se_rule_deleted_questions');
						} else {
							echo implode('<br/><b>'.$rule_logic.'</b>', $descriptions);
						}
						echo '<br/>';

					}//end else rules not empty

					echo 'Send a <b>';
					if ($info['send'] == 'staf') {
						echo translate('cms_form_staf');
					} else if ($info['send'] == 'recipient') {
						echo translate('cms_form_recipient');
					} else if ($info['send'] == 'receipt') {
						echo translate('cms_form_receipt');
					} else {
						echo translate('cms_form_unknown_email_type');
						$valid = FALSE;
					}

					echo '</b> email to <b>';
					if (empty($info['address'])) {
						$valid = FALSE;
						echo translate('cms_form_no_email_address');
					} else {
						echo $info['address'];
					}
					echo '</b>';
					?>
				</td>
				<td valign="top" align="center" width="70" class="sq-backend-table-cell"><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $valid ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /></td>
				<?php
				if ($admin_access) {
					?><td valign="top"  align="center" width="50" class="sq-backend-table-cell"><?php check_box($prefix.'_deletes[]', $k); ?></td><?php
				}
				?>
			</tr>
			<?php
		}//end for all selective emails
		?>
		</table>
		<?php

		return TRUE;

	}//end paintCurrentSelectiveEmails()


	/**
	* Processes the interface for deleting current selective email rules
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_deletes'])) return TRUE;

		$all_se = $asset->attr('selective_emails');
		$delete_ruleids = $_POST[$prefix.'_deletes'];

		// delete our selective e-mail rules
		foreach ($delete_ruleids as $ruleid) {
			unset($all_se[$ruleid]);
			if (isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
				if ($ruleid == $_REQUEST[$prefix.'_se_active_ruleid']) {
					unset($_REQUEST[$prefix.'_se_active_ruleid']);
				}
			}
		}

		$asset->setAttrValue('selective_emails', $all_se);

		return $asset->saveAttributes();

	}//end processCurrentSelectiveEmails()


	/**
	* Processes the interface for selecting a question for a rule part
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintActiveQuestion(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		if (isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			$current_assetid = isset($_POST[$prefix.'_se_new_qid']['assetid']) ? $_POST[$prefix.'_se_new_qid']['assetid'] : 0;
			asset_finder($prefix.'_se_new_qid', $current_assetid, Array('form_question' => 'D'));
			return TRUE;
		} else {
			return FALSE;
		}

	}//end paintActiveQuestion()


	/**
	* Processes the interface for selecting active question (no implementation required)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processActiveQuestion(&$asset, &$o, $prefix)
	{
		return TRUE;

	}//end processActiveQuestion()


	/**
	* Paint the dropdowns to add rule parts to a Selective Email rule
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddRulePart(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return FALSE;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			return FALSE;
		}

		// we cleared the asset finder for adding a new rule part
		if (isset($_POST[$prefix.'_se_new_qid']) && $_POST[$prefix.'_se_new_qid']['assetid'] == 0) {
			unset($_POST[$prefix.'_se_new_qid']['assetid']);
		}

		if (!isset($_POST[$prefix.'_se_new_qid']['assetid'])) {
			echo translate('cms_form_select_question_before_part');
			return FALSE;
		} else {
			$active_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_se_new_qid']['assetid']);

			$rule_codes = $active_question->getAllowedRules();
			$type_codes = Array();
			foreach ($rule_codes as $rule_code) {
				$type_code = 'form_question_rule_type_'.$rule_code;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$se_rule = eval('return '.$type_code.'::isSelectiveEmailRule();');
				if ($se_rule) $type_codes[] = $type_code;
			}

			$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');
			combo_box($prefix.'_new_rule', array_merge(Array(' ' => '------ '.translate('please_select').' ------'), $desc), FALSE, '');

			return TRUE;
		}

	}//end paintAddRulePart()


	/**
	* Process the interface for adding a rule part to a Selective Email rule
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRulePart(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return FALSE;

		// rule id is not set || required "new rule part" fields not set = FAIL
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid']) ||
			!isset($_REQUEST[$prefix.'_new_rule']) || empty($_POST[$prefix.'_se_new_qid']['assetid'])) {
			return FALSE;

		} else {
			$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
			$qid = $_POST[$prefix.'_se_new_qid']['assetid'];
			$active_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($qid);

			$type_code = $_POST[$prefix.'_new_rule'];
			$rule_code = substr($type_code,24);
			if (!trim($rule_code)) return FALSE;

			$all_se = $asset->attr('selective_emails');

			$rules =& $all_se[$ruleid]['rules'];

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$operators = array_keys(eval('return '.$type_code.'::getOperators();'));

			$rules[] = Array(
						'assetid'					=> $qid,
						'name'						=> $rule_code,
						'operator'					=> $operators[0],
						'value'						=> '',
						'comparison_question_id'	=> 0,
					   );


			$asset->setAttrValue('selective_emails', $all_se);
			return $asset->saveAttributes();
		}

	}//end processAddRulePart()


	/**
	* Paints the interface for editing and deleting Rule Parts
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRuleParts(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			$o->openField('Note');
			echo translate('cms_form_unlock_to_select_se_rule');
			$o->closeField();
			return FALSE;
		}

		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			$o->openField('Note');
			echo translate('cms_form_select_to_edit_se_rule');
			$o->closeField();
			return FALSE;
		}

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];

		$write_access = $asset->writeAccess('attributes');

		$am		= $GLOBALS['SQ_SYSTEM']->am;
		$all_se	= $asset->attr('selective_emails');
		$rules  = $all_se[$ruleid]['rules'];
		$descs  =  Array();

		if (empty($rules)) {
			$o->openField('Note');
			echo translate('cms_form_se_rule_empty');
			$o->closeField();
			return FALSE;
		}

		for (reset($rules); NULL !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			// let the rule paint itself here!!
			$type_code = 'form_question_rule_type_'.$rule['name'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();
			$edit_fns = $rule_asset->getEditFns();

			if (!isset($descs[$type_code])) {
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(Array($type_code), 'description');
				$descs[$type_code] = $asset_info[$type_code];
			}

			$o->openSection($descs[$type_code]);

			$o->openField(translate('applies_to'));
				$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($rule['assetid']);
				if ($q_asset) {
					echo get_asset_tag_line($q_asset->id);
				} else {
					echo '<i class="sq-backend-warning">'.translate('cms_form_deleted_question_cannot_edit', $rule['assetid']).'</i>';
				}
			$o->closeField();

			if ($q_asset) {
				$edit_fns->paintRule($rule_asset, $o, $prefix.'_rule_'.$k, $rule, $write_access, $q_asset);
			}

			if ($write_access) {
				$o->openField('Delete ?');
					check_box($prefix.'_rule_'.$k.'[delete]');
				$o->closeField();
			}

			$o->closeSection();

		}//end for all rules

		return $write_access;

	}//end paintRuleParts()


	/**
	* Processes the interface for editing and deleting Rule Parts
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRuleParts(&$asset, &$o, $prefix)
	{
		$all_se = $asset->attr('selective_emails');
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			return FALSE;
		}

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		if (!isset($all_se[$ruleid])) return FALSE;

		$rules =& $all_se[$ruleid]['rules'];

		$prefix = $asset->getPrefix();

		// delete rules first
		if (empty($rules)) $rules = Array();
		foreach (array_keys($rules) as $k) {
			$rule_prefix = $prefix.'_rule_'.$k;
			if (!isset($_POST[$rule_prefix])) continue;

			if (array_get_index($_POST[$rule_prefix], 'delete', 0)) {
				unset($rules[$k]);
			}
		}

		// modify what's left
		for (reset($rules); NULL !== ($k = key($rules)); next($rules)) {
			$rule =& $rules[$k];
			// let the rule paint itself here!!
			$type_code = 'form_question_rule_type_'.$rule['name'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$rule_asset = new $type_code();
			$edit_fns = $rule_asset->getEditFns();

			$edit_fns->processRule($rule_asset, $o, $prefix.'_rule_'.$k, $rule);
		}

		$asset->setAttrValue('selective_emails', $all_se);
		return $asset->saveAttributes();

	}//end processRuleParts()


	/**
	* Paint the dropdowns etc for the rule logic
	*
	* When <all|any> rule parts are matched, send an <email type> email to <email address>
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintActiveSend(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return FALSE;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			return FALSE;
		}

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		$all_se = $asset->attr('selective_emails');

		// we are displaying these as substitutions into the next translation
		ob_start();
			combo_box($prefix.'_require', Array('all' => 'ALL', 'any' => 'ANY'), FALSE, Array($all_se[$ruleid]['require']));
			$require_box = ob_get_contents();
		ob_end_clean();

		ob_start();
			combo_box($prefix.'_send', Array('recipient' => 'Recipient E-mail', 'receipt' => 'Receipt', 'staf' => 'Send To A Friend E-mail'), FALSE, Array($all_se[$ruleid]['send']));
			$email_type_box = ob_get_contents();
		ob_end_clean();

		ob_start();
			// requested in bug#1437 - changed input to a text area
			text_area($prefix.'_address', $all_se[$ruleid]['address'], 30, 5);
			$address_box = ob_get_contents();
		ob_end_clean();

		echo translate('cms_form_match_email_line', $require_box, $email_type_box, $address_box);


		return TRUE;

	}//end paintActiveSend()


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
	function processActiveSend(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return FALSE;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid']) || !isset($_POST[$prefix.'_address'])) {
			return FALSE;
		}

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		$all_se = $asset->attr('selective_emails');

		$all_se[$ruleid]['require'] = $_POST[$prefix.'_require'];
		$all_se[$ruleid]['send']    = $_POST[$prefix.'_send'];
		$all_se[$ruleid]['address'] = $_POST[$prefix.'_address'];

		$asset->setAttrValue('selective_emails', $all_se);
		return $asset->saveAttributes();

	}//end processActiveSend()


	/**
	* Paint a note giving instructions for complex formatting
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintKeywordReplacementText(&$asset, &$o, $prefix)
	{
		?>
		<p>In addition to the simple formatting options shown underneath each email, you can use the same keyword replacements as is available for the Thank You bodycopy. Click here for a <a href="#" onClick="javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/form/popups/thank_you_keywords.php?assetid=<?php echo $asset->id ?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520')">list of keywords available for use in emails</a>.<br/></p>

		<?php
		return TRUE;

	}//end paintKeywordReplacementText()


	/**
	* Process the note giving instructions for keyword replacements
	*
	* Nothing to process, so return false always.
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processKeywordReplacementText(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processKeywordReplacementText()


	//--        FORM ACTIONS        --//


	/**
	* Paints the interface for printing all existing actions
	*
	* @param Form_Email			$asset	The form whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintAllActions(Form_Email $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');

		// Get selected action, which may have changed last commit
		if ($write_access) {
			$selected_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
			if (!is_numeric($selected_action)) {
				$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
			}
		} else {
			$selected_action = NULL;
		}

		$o->openField('');

			hidden_field($prefix.'_new_selected_action', '');

			if (!empty($actions)) {
				?><table class="sq-backend-table">
				<colgroup>
					<col width="60"  align="center" />
					<col width="150" />
					<col/>
					<col width="60"  align="center" />
					<col width="60"  align="center" />
				</colgroup>
				<thead>
					<th><p>Edit</p></th>
					<th><p>Action Name / Type</p></th>
					<th><p>Summary</p></th>
					<th><p>Valid ?</p></th>
					<th><p>Active ?</p></th>
					<th><p>Delete ?</p></th>
				</thead><tbody><?php

				foreach ($actions as $key => $action) {
					$is_selected_action = (!is_null($selected_action) && ((string)$selected_action === (string)$key));
					?><tr<?php
					if ($is_selected_action) {
						echo ' class="alt"';
					}
					?>><?php
					$action_type = $action['type_code'];
					$settings    = $action['settings'];
					$type_name   = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_type);

					if ($write_access && !$is_selected_action) {
						?><td class="sq-backend-table-cell"><p><a href="#" onclick="document.getElementById('<?php echo $prefix ?>_new_selected_action').value = <?php echo $key ?>; document.main_form.submit(); return false;"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a></p></td><?php
					} else {
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?><td class="sq-backend-table-cell"><p><strong><?php echo $action['name'] ?></strong><br/>(<?php echo $type_name ?>)</p></td><?php
					?><td class="sq-backend-table-cell"><?php echo call_user_func(Array($action_type, 'paintSummary'), $asset, $settings, $o, $prefix, $key) ?></td><?php
					?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo call_user_func(Array($action_type, 'isValid'), $asset, $settings) ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
					if ($write_access) {
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[active]['.$key.']', '1', $action['active']) ?></p></td><?php
						?><td class="sq-backend-table-cell"><p><?php check_box($prefix.'_rules[delete]['.$key.']', '1', FALSE) ?></p></td><?php
					} else {
						?><td class="sq-backend-table-cell"><p><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $action['active'] ? 'tick' : 'cross' ?>.gif" width="15" height="15" border="0" /></p></td><?php
						?><td class="sq-backend-table-cell">&nbsp;</td><?php
					}
					?></tr><?php
				}
				?></tbody></table><?php
			} else {
				?><p>There are no actions currently defined for this form.</p><?php
			}

		$o->closeField();
		$o->sectionNote('Only actions that are <strong>valid</strong> and <strong>active</strong> will be executed when a form is submitted. An action can be activated or deactivated with the <strong>Active ?</strong> check box.');

		return $write_access;

	}//end paintAllActions()


	/**
	* Performs processing for the actions table
	*
	* @param Form_Email			$asset	The form whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processAllActions(Form_Email $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		// If we have selected "edit action", then we don't want this to run,
		// because we only want active/delete changes to take effect upon
		// hitting the commit button.
		$edit_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (!empty($edit_action)) {
			return $write_access;
		}

		if ($write_access) {
			$actions = $asset->attr('actions');

			$rule_changes = array_get_index($_POST, $prefix.'_rules', Array());

			if(!empty($actions)) {
				foreach ($actions as $key => $action) {
					if (!isset($rule_changes['active'][$key])) {
						$rule_changes['active'][$key] = Array();
					}
				}
			} else {
				if (!isset($rule_changes['active'])) {
					$rule_changes['active'] = Array();
				}
			}

			if (!isset($rule_changes['delete'])) {
				$rule_changes['delete'] = Array();
			}

			foreach ($rule_changes['active'] as $key => $value) {
				$actions[$key]['active'] = (boolean)$value;
			}
			foreach ($rule_changes['delete'] as $key => $value) {
				unset($actions[$key]);
			}

			$asset->setAttrValue('actions', $actions);
			if (!$asset->saveAttributes()) return FALSE;
		}

		return $write_access;

	}//end processAllActions()


	/**
	* Paints the interface for creating a new action
	*
	* @param Form_Email			$asset	The form whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintNewAction(Form_Email $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('form_action');
		$names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($types, 'name');
		$o->openField('');
			if ($write_access) {
				?><p><?php
					check_box($prefix.'_new', '1', FALSE, 'this.form.'.$prefix.'_new_type.disabled = !this.checked; return true;');
					?><label for="<?php echo $prefix ?>_new">Create a new action of type:</label> <?php
					$names = Array('' => 'Select action type...') + $names;
					combo_box($prefix.'_new_type', $names, FALSE, '', 0, 'disabled="disabled"');
					?></p><?php
			} else {
				?><p>This screen must be locked before you can create a new action.</p><?php
			}

			// If we have a new action, are we
		$o->closeField();

		$o->sectionNote('If you create a new action, the new action will be displayed immediately for you to edit. If you already have an action open to edit, the changes to that action will be saved when you commit before the new action is created.');

		return $write_access;

	}//end paintNewAction()


	/**
	* Processes the creation of a new action
	*
	* @param Form_Email			$asset	The form whose interface we are processing
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processNewAction(Form_Email $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			// First check that we are creating a new rule.
			$new_action = (int)array_get_index($_POST, $prefix.'_new', '0');

			if ($new_action) {
				// Paranoia, paranoia...
				// Check whether the type we've been passed is a valid type,
				// to protect possible stuffed POSTDATA (including a blank)
				$action_type = array_get_index($_POST, $prefix.'_new_type', '');
				if (!empty($action_type)) {
					if ($GLOBALS['SQ_SYSTEM']->am->installed($action_type)) {
						// Also check whether it's a valid form action
						$type_parents = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($action_type);
						if (in_array('form_action', $type_parents)) {
							// okay, we seem to be in the clear
							$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action_type, 'name');
							$actions   = $asset->attr('actions');

							if (!empty($actions)) {
								$new_key = max(array_keys($actions)) + 1;
							} else {
								$new_key = 0;
							}

							$actions[$new_key] = Array(
													'name'		=> $this->_makeValidActionName($asset, $type_name),
													'type_code'	=> $action_type,
													'settings'	=> NULL,
													'active'	=> TRUE,
												 );
							$asset->setAttrValue('actions', $actions);

							// Set new selected action
							$_POST[$prefix.'_new_selected_action'] = $new_key;

							if (!$asset->saveAttributes()) return FALSE;
						} else {
							trigger_error('Cannot create new action; asset type "'.$action_type.'" is not a form action', E_USER_WARNING);
						return FALSE;
						}
					} else {
						trigger_error('Cannot create new action; "'.$action_type.'" is not a valid asset type', E_USER_WARNING);
						return FALSE;
					}
				}
			}

		}

		return $write_access;

	}//end processNewAction()


	/**
	* Paints the interface for the selected submission action
	*
	* @param Form_Email			$asset	The form whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function paintSelectedAction(Form_Email $asset, Backend_Outputter $o, $prefix)
	{	
		$write_access = $asset->writeAccess('attributes');
		$selected_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (!is_numeric($selected_action)) {
			$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$actions = $asset->attr('actions');

				// Get the current action and process - but not if we've
				// already been deleted further up!
				if (!isset($actions[$selected_action])) {
					$o->openField('');
						?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
					$o->closeField();
					return FALSE;
				}

				$action = $actions[$selected_action];

				$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');

				$o->openField('Action Name');

					text_box($prefix.'_selected_name', $action['name'], 30);
					$o->note('An optional name to identify this particular action. If cleared, the action\'s type ("'.$type_name.'" in this case) will be used, numerically indexed if necessary.');

				$o->closeField();
				$o->openField('Settings');

					hidden_field($prefix.'_selected_action', $selected_action);
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
					call_user_func(Array($action['type_code'], 'paintInlineInterface'), $asset, $action['settings'], $o, $prefix);

				$o->closeField();

			} else {
				$o->openField('');
					?><p>No action is currently selected for editing. To edit an action, please select from the "Existing Actions" list above.</p><?php
				$o->closeField();

			}//end if there is a selected action
		} else {
			$o->openField('');
				?><p>This screen must be locked before you can edit an action.</p><?php
			$o->closeField();
		}//end if write_access

		return $write_access;

	}//end paintSelectedAction()


	/**
	* Processes the interface for the selected action
	*
	* @param Form_Email			$asset	The form whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	*/
	public function processSelectedAction(Form_Email $asset, Backend_Outputter $o, $prefix)
	{	
		$write_access = $asset->writeAccess('attributes');
		$actions = $asset->attr('actions');
		$result = FALSE;

		// If an "edit action" has been clicked, then we've lost the changes
		// to the current rule.
		$edit_action = array_get_index($_REQUEST, $prefix.'_new_selected_action', NULL);
		if (is_numeric($edit_action)) {
			return FALSE;
		}

		// If not, then get the current action and process - but not if we've
		// already been deleted!
		$selected_action = array_get_index($_POST, $prefix.'_selected_action', NULL);
		if (!isset($actions[$selected_action])) {
			return FALSE;
		}

		if ($write_access) {
			if (!is_null($selected_action)) {
				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				$action = $actions[$selected_action];

				$new_name = array_get_index($_POST, $prefix.'_selected_name', NULL);
				if ($new_name != $action['name']) {
					if (empty($new_name)) {
						$new_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($action['type_code'], 'name');
					}
					$actions[$selected_action]['name'] = $this->_makeValidActionName($asset, $new_name);
				}

				$GLOBALS['SQ_SYSTEM']->am->includeAsset($action['type_code']);
				$settings = $action['settings'];

				// Bug Fix #3145
				// call_user_func doesn't work with pass-by-reference in PHP5, so using call_user_func_array instead
				$result = call_user_func_array(Array($action['type_code'], 'processInlineInterface'), Array($asset, &$settings, $o, $prefix));
				if ($result) {
					$actions[$selected_action]['settings'] = $settings;
					$asset->setAttrValue('actions', $actions);
					if (!$asset->saveAttributes()) return FALSE;
				}
			} else {
				return FALSE;
			}
		}

		return ($result && $write_access);

	}//end processSelectedAction()


	/**
	* Make an action name valid by adding a numeric index to it if a duplicate
	*
	* Returns the base name if no duplicate name is found, or tries "Base Name
	* 2", "Base Name 3", and so on, until a unique name is found, and returns
	* the first unique name it finds.
	*
	* @param Form_Email			$asset		The form whose interface we are painting
	* @param string				$base_name	Base name to use
	*
	* @return boolean
	*/
	protected function _makeValidActionName(Form_Email $asset, $base_name)
	{
		$actions = $asset->attr('actions');

		$name = $base_name;
		$i = 1;

		do {
			$found = FALSE;

			foreach ($actions as $action) {
				if ($action['name'] == $name) {
					$found = TRUE;

					// Incrememt the base name
					$i++;
					$name = $base_name.' '.$i;
					break;
				}
			}

		} while ($found);

		return $name;

	}//end _makeValidActionName()
	

}//end class
?>
