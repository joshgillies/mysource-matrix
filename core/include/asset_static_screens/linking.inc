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
* $Id: linking.inc,v 1.28.2.2 2006/12/29 05:41:05 tbarrett Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Linking Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.28.2.2 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing links to/from this asset
*
* @param object		&$owner		the asset whose interface we are painting
* @param object		&$o			the outputter class
* @param object		&$ei		the edit interface
* @param boolean	$allow_edit	allow editing of link types if user has access
*
* @return boolean
* @access public
*/
function paintLinking(&$owner, &$o, &$ei, $allow_edit=TRUE)
{
	// If we are allowed to edit, we check the write effective access, minus live edit access
	// We are ignoring live edit access because changes to links are not reliant on
	// status of the asset (because the are not safe edited)
	$created = FALSE;

	$write_access = ($allow_edit) ? $owner->writeAccess('links') :  FALSE;

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
	$o->openSection(translate('current_linking'));
	$o->openField('');
	echo '<i>'.translate('link_types_note').'</i><br/><br/>';

	$db =& $GLOBALS['SQ_SYSTEM']->db;
	$assetids = Array();
	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor');

	$editable_link_types = Array(
							SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
							SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
						   );

	foreach ($asset_links as $link) {
		$assetids[$link['majorid']] = Array();
	}

	$total_assets = 0;
	$num_per_page = 50;
	$link_offset = array_get_index($_POST, $prefix.'_link_offset', 0);

	if (empty($assetids)) {
		$result = Array();
	} else {
		$result = Array();

		$assetid_keys = array_keys($assetids);
		$total_assets = count($assetid_keys);
		$assetid_keys = array_slice($assetid_keys, $link_offset, $num_per_page);
		// Chunk Norris this query because the IN part will bomb with >1000
		// assets on Oracle
		$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, pt.treeid as parent_treeid, a.name, a.assetid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ON cl.linkid = ct.linkid,
					 '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk pl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt ON pl.linkid = pt.linkid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON a.assetid = pl.minorid
				';
		foreach ($assetid_keys as $assetid_key_index => $assetid_key) {
			$assetid_keys[$assetid_key_index] = $db->quoteSmart((string) $assetid_key);
		}
		$where = 'cl.minorid IN ('.implode(', ', $assetid_keys).')
				  AND ct.treeid LIKE pt.treeid || '.$db->quote('%').'
				  AND pt.treeid <= ct.treeid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.' ORDER BY cl.linkid, ct.treeid, pt.treeid';

		$result = $db->getAssoc($sql, FALSE, Array(), DB_FETCHMODE_ASSOC, TRUE);

		assert_valid_db_result($result);
	}

	foreach ($asset_links as $link_data) {
		$link_info[$link_data['majorid']][] = $link_data;
	}

	$output_lineages = Array();
	$editing_linkids = Array();
	foreach ($result as $treeid => $link_tree) {

		// work out some values we are going to need
		$parent_data = $link_tree[count($link_tree) -1];
		$majorid = $parent_data['assetid'];

		foreach ($link_info[$majorid] as $link_data) {

			$link_type = $link_data['link_type'];
			$linkid = $link_data['linkid'];

			// work out if the user can change the link types
			// the arg $allow_edit must be set to TRUE to try the write access test
			$can_edit = ($write_access && in_array($link_type, array_keys($editable_link_types)) && !in_array($linkid, $editing_linkids));

			$asset_lineage = '';
			$asset_lineage_ids = Array();
			foreach ($link_tree as $tree_data) {
				$asset_lineage_ids[] = $tree_data['assetid'];
				if ($tree_data['assetid'] == $majorid) continue;
				$asset_lineage .= $tree_data['name'].' &gt; ';
			}
			$asset_lineage_ids[] = $owner->id;

			$asset_lineage .= '<b>'.$parent_data['name'].'</b>';
			if (isset($output_lineages[$asset_lineage]) && !$can_edit) {
				continue;
			}

			ob_start();
				?>
				<tr>
					<td class="sq-backend-table-cell" style="text-align: center;">
						<?php
						if ($link_type & SQ_SC_LINK_BACKEND_NAV) {
							?>
							<a href="#">
							<?php echo sq_print_icon(sq_web_path('lib').'/web/images/icons/asset_locator.png', 16, 16, translate('show_in_asset_map'), translate('show_in_asset_map'),' style="border: 0; cursor: pointer"  onclick="'.asset_locator_js($asset_lineage_ids).'"');
							?>
							</a>
							<?php
						}
						?>
					</td>
					<td style="text-align: center;">
						<?php echo $linkid; ?>
					</td>
					<td style="text-align: center;">
						<?php
						if (!$can_edit) {
							echo link_type_name($link_type);
						} else {
							combo_box($prefix.'_link_type['.$linkid.']', $editable_link_types, FALSE, $link_type);
							$editing_linkids[] = $linkid;
						}
						?>
					</td>
					<td style="text-align: center;">
						<?php echo $majorid; ?>
					</td>
					<td><?php echo $asset_lineage; ?></td>
				</tr>
				<?php
				$output_lineages[] = ob_get_contents();
			ob_end_clean();
		}//end foreach

	}//end foreach result

	if (!count($output_lineages)) {
		echo 'None';
	} else {
		$num_pages = ceil($total_assets / $num_per_page);
		?>
		<style type="text/css">
				td.sq-linking-header {
					padding-top: 0px;
				}
				.sq-linking-header table.sq-backend-table {
					border-top: 0px;
					margin: 0 8px 8px 8px;
				}
				.sq-linking-header table.sq-backend-table th {
					font-weight: normal;
					text-decoration: italic;
				}
				table.sq-backend-table tr td.sq-linking-header-header {
					background-color: #666;
					color: #fff;
					font-size: 11px;
				}
			</style>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-linking-header-header" colspan="5"><b><?php
				hidden_field($prefix.'_link_offset', $link_offset);
				if ($link_offset > 0) {
					?><a title="<?php echo translate('pagination_go_to_first') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
				}
				?>
				&lt;&lt;
				<?php
				if ($link_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if ($link_offset > 0) {
					?><a title="<?php echo translate('pagination_go_to_previous') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('pagination_cannot_go_further_back') ?>" style="color: #333"><?php
				}
				?>
				&lt;
				<?php
				if ($link_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?>
				&nbsp; &nbsp; <?php echo translate('page_number', round(($link_offset + $num_per_page) / $num_per_page), ceil($total_assets / $num_per_page)); ?> &nbsp; &nbsp;
				<?php
				if (($link_offset + $num_per_page) < $total_assets) {
					?><a title="<?php echo translate('pagination_go_to_next') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo $link_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
				}
				?>
				&gt;
				<?php
				if (($link_offset + $num_per_page) < $total_assets) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if (($link_offset + $num_per_page) < $total_assets) {
					?><a title="<?php echo translate('pagination_go_to_last') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_link_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
				} else {
					?><span title="<?php echo translate('pagination_cannot_go_further_forward') ?>" style="color: #333"><?php
				}
				?>
				&gt;&gt;
				<?php
				if (($link_offset + $num_per_page) < $total_assets) {
					?></a><?php
				} else {
					?></span><?php
				}
				?></b>
			</td>
			</tr>
			<tr>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('find'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('link_id'); ?></th>
				<th style="text-align: center;" width="10%"><?php echo translate('link_type'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('assetid'); ?></th>
				<th width="70"><?php echo translate('lineage'); ?></th>
			</tr>
			<?php
				foreach ($output_lineages as $null => $lineage_data) {
					echo $lineage_data;
				}
			?>
		</table>
		<?php
	}//end else

	$o->closeSection();

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');
	$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);

	if (!empty($fl_jobs) || $write_access) {
		$created = TRUE;
		$o->openSection(translate('future_linking'));
			if (!empty($fl_jobs)) {
				$o->openField('', 'new_line');
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header">
								<?php echo translate('action'); ?>
							</td>
							<td class="sq-backend-table-header">
								<?php echo translate('when'); ?>
							</td>
							<?php
							if ($write_access) {
								$created = TRUE;
								?>
								<td class="sq-backend-table-header" style="text-align: center;"><?php echo translate('delete_question'); ?></td>
								<?php
							}
							?>
						</tr>
						<?php
						for ($i = 0; $i < count($fl_jobs); $i++) {
							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php echo nl2br($fl_jobs[$i]->readableDescription()); ?>
								</td>
								<td class="sq-backend-table-cell">
									<?php echo $fl_jobs[$i]->readableWhen(); ?>
								</td>
								<?php
								if ($write_access) {
									$created = TRUE;
									?>
									<td class="sq-backend-table-cell" style="text-align: center;">
										<?php
										if ($fl_jobs[$i]->canDelete()) {
											check_box($prefix.'_remove_fl[]', $fl_jobs[$i]->id);
										} else {
											echo '&nbsp;';
										}
										?>
									</td>
									<?php
								}
								?>
							</tr>
							<?php
						}//end for
						?>
					</table>
					<br/>
					<?php
				$o->closeField();
			}//end if future lineage jobs

			if ($write_access) {
				$created = TRUE;
				$fl =& new Cron_Job_Future_Lineage();
				$fl->setAssetInLink($owner, 'minor');
				$edit_fns = $fl->getEditFns();
				$edit_fns->paintInLineBackend($fl, $o, $prefix.'_new_fl');
			}

		$o->closeSection();

	}//end if future lineage jobs or write access

	return $created;

}//end paintLinking()


/**
* Processes the interface for viewing links to/from this asset
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processLinking(&$owner, &$o, &$ei)
{
	// if we dont have admin access, go away
	if ($owner->id && !$owner->writeAccess('')) {
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $owner->name));
		exit();
	}

	// if we dont have effective write access, dont process anything
	if (!$owner->writeAccess('links')) return FALSE;

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$done_something = FALSE;
	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor');

	foreach ($asset_links as $link) {
		if (!isset($_POST[$prefix.'_link_type'][$link['linkid']])) {
			continue;
		}
		$new_type = $_POST[$prefix.'_link_type'][$link['linkid']];
		if ($link['link_type'] == $new_type) continue;
		if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], $new_type)) {
			continue;
		}
		$done_something = TRUE;
	}

	// Refresh the Asset Map to show linking changes
	if ($done_something) {
		$o->addOnLoad('if (window.top.frames["sq_sidenav"] && window.top.frames["sq_sidenav"].reload_asset) window.top.frames["sq_sidenav"].reload_asset("'.$owner->id.'");');
	}

	$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');

		if (!empty($_POST[$prefix.'_remove_fl'])) {
			$fl_jobs = &Cron_Job_Future_Lineage::getActiveJobs($owner);
			if (!empty($fl_jobs)) {

				if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
					for ($i = 0; $i < count($fl_jobs); $i++) {
						if (in_array($fl_jobs[$i]->id, $_POST[$prefix.'_remove_fl'])) {
							if ($cron_mgr->removeJob($fl_jobs[$i])) {
								$is_updated = TRUE;
							}
						}
					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

				} else {
					trigger_localised_error('SYS0232', E_USER_NOTICE);
				}

			}// end if
		}// end if

		$fl =& new Cron_Job_Future_Lineage();
		$edit_fns = $fl->getEditFns();
		if ($fl->setAssetInLink($owner, 'minor') && $edit_fns->processInLineBackend($fl, $o, $prefix.'_new_fl')) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			if ($cron_mgr->addJob($fl, $GLOBALS['SQ_SYSTEM']->user)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$done_something = TRUE;
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

	}//end if

	return $done_something;

}//end processLinking()


?>
