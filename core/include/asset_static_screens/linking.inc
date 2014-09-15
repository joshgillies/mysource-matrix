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
* $Id: linking.inc,v 1.52 2013/10/22 00:38:23 lwright Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Linking Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.52 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing links to/from this asset
*
* @param object		&$owner			the asset whose interface we are painting
* @param object		&$o			the outputter class
* @param object		&$ei			the edit interface
* @param array|NULL	$static_keywords	the wanted keyword of this static screen
*
* @return boolean
* @access public
*/
function paintLinking(&$owner, &$o, &$ei, $static_keywords=NULL)
{
	// We are ignoring live edit access because changes to links are not reliant on
	// status of the asset (because the are not safe edited)
	$created = FALSE;

	$write_access = $owner->writeAccess('links');

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
	$o->openSection(translate('current_linking'));

	// which display type?
	$o->openField(translate('display_linking_type'));

	$display_types = Array(
							'display_linked_parents'	=> translate('display_linked_parents'),
							'display_linked_children'	=> translate('display_linked_children'),
							);
	$current_display_type = isset($_POST[$prefix.'_display_type']) ? $_POST[$prefix.'_display_type'] : 'display_linked_parents';
	echo hidden_field($prefix.'_display_type_changed', '0');
	$display_reverse = ($current_display_type == 'display_linked_children');
	combo_box($prefix.'_display_type', $display_types, FALSE, $current_display_type, 0 , 'onChange="this.form.'.$prefix.'_display_type_changed.value=1;submit_form(this.form)"');

	$o->closeField();


	$o->openField('');
	echo '<i>'.translate('link_types_note').'</i><br/><br/>';

	$db = MatrixDAL::getDb();
	$assetids = Array();


	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, $display_reverse ? 'major' : 'minor');
	foreach ($asset_links as $link) {
		$assetids[$link[$display_reverse ? 'minorid' : 'majorid']] = Array();
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
		$sql = 'SELECT ct.treeid as our_treeid, cl.minorid, cl.linkid, pt.treeid as parent_treeid, a.name, a.assetid, cl.value
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk cl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree ct ON cl.linkid = ct.linkid,
					 '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk pl
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk_tree pt ON pl.linkid = pt.linkid
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast a ON a.assetid = pl.minorid
				';

		$bindVars = array();
		foreach ($assetid_keys as $assetid_key_index => $assetid_key) {
			$assetid_keys[$assetid_key_index] = ':assetid_keys_'.$assetid_key_index;
			$bindVars['assetid_keys_'.$assetid_key_index] = (string) $assetid_key;
		}
		$where = ' where cl.minorid IN ('.implode(', ', $assetid_keys).')
				  AND ct.treeid LIKE pt.treeid || \'%\'
				  AND pt.treeid <= ct.treeid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pl');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.' ORDER BY cl.sort_order, cl.linkid, ct.treeid, pt.treeid';

		try {
			$query  = MatrixDAL::preparePdoQuery($sql);
			foreach ($bindVars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoGroupedAssoc($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

	}//end else


	foreach ($asset_links as $link_data) {
		$link_info[$link_data[$display_reverse ? 'minorid' : 'majorid']][] =$link_data;
	}


	$output_lineages = Array();
	$editing_linkids = Array();
	foreach ($result as $treeid => $link_tree) {
		// work out some values we are going to need
		$parent_data = $link_tree[count($link_tree) -1];
		$majorid = $parent_data['assetid'];
		$linkid = $parent_data['linkid'];

		foreach ($link_info[$majorid] as $link_data) {
			// to display linked children, we don't want to show entire lineage of children, just show lineage via current asset
			if($display_reverse && $linkid != $link_data['linkid'] && $link_data['link_type'] != SQ_LINK_NOTICE) continue;

			$output = _printLine($owner, $majorid, $link_data, $parent_data, $link_tree, $editing_linkids, $display_reverse);
			if (!empty($output)) $output_lineages[] = $output;
		}//end foreach

	}//end foreach result



	if (!count($output_lineages)) {
		echo 'None';
	} else {
		$num_pages = ceil($total_assets / $num_per_page);
		?>
		<table class="sq-backend-table">
		<tr>
			<th class="sq-pagination-links" colspan="7"><?php
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
				?>
			</th>
			</tr>
			<tr>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('find'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('link_id'); ?></th>
				<th style="text-align: center;" width="10%"><?php echo translate('link_type'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('assetid'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo translate('value'); ?></th>
				<th style="text-align: center; white-space: nowrap;" width="5%"><?php echo ucwords(strtolower(translate('locked'))); ?></th>
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
	$fl_jobs = call_user_func(Array('Cron_Job_Future_Lineage', 'getActiveJobs'), $owner, $display_reverse);

	if (!empty($fl_jobs)) {
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

		$o->closeSection();



	}//end if future lineage jobs or write access

	// if the user got the write access we add two sections
	// create linking and delete linking
	if ($write_access) {
		// use the cron Job future linage asset to print the
		// create new link screen

		$fl_create = new Cron_Job_Future_Lineage();
		$fl_create->setAssetInLink($owner, $display_reverse ? 'major' : 'minor');
		$edit_fns_create = $fl_create->getEditFns();
		if($display_reverse){
			$edit_fns_create->paintInLineBackendCreateReverse($fl_create, $o, $prefix.'_create_reverse');
			$edit_fns_create->paintInLineBackendDeleteReverse($fl_create, $o, $prefix.'_delete_reverse');
		}
		else {
			$edit_fns_create->paintInLineBackendCreate($fl_create, $o, $prefix.'_create');
			$edit_fns_create->paintInLineBackendDelete($fl_create, $o, $prefix.'_delete');
		}
		$created = TRUE;

	}
	return $created;
	return TRUE;

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
		$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));
		exit();
	}

	// if we dont have effective write access, dont process anything
	if (!$owner->writeAccess('links')) return FALSE;

	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$current_display_type = isset($_POST[$prefix.'_display_type']) ? $_POST[$prefix.'_display_type'] : 'display_linked_parents';
	$display_reverse = ($current_display_type == 'display_linked_children');

	$done_something = FALSE;

	$asset_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, $display_reverse ? 'major'  : 'minor');
	// process link_type changes
	foreach ($asset_links as $link) {
		// Find out the type
		if (!isset($_POST[$prefix.'_link_type'][$link['linkid']])) {
			$new_type = NULL;
		} else {
			$new_type = $_POST[$prefix.'_link_type'][$link['linkid']];
			if ($link['link_type'] == $new_type) {
			$new_type = NULL;
			}
		}

		// Find out the value
		if (!isset($_POST[$prefix.'_link_value'][$link['linkid']])) {
			$new_value = NULL;
		} else {
			$new_value = $_POST[$prefix.'_link_value'][$link['linkid']]; // Set the value for testing
			if (html_entity_decode($link['value'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET) == $new_value) {
				$new_value = NULL;
			}
		}

		// Find out the value
		if (!isset($_POST[$prefix.'_link_locked'][$link['linkid']])) {
			$new_lock_status = '0';
		} else {
			$new_lock_status = '1'; // Set the value for testing
		}//end if
		$new_lock_status = ($new_lock_status == '1') ? '1' : '0';
		if ((isset($_POST[$prefix.'_display_type_changed']) && $_POST[$prefix.'_display_type_changed'] == 1) || $link['locked'] == $new_lock_status) {
			$new_lock_status = NULL;
		}//end if

		// Nothing to do let's continue
		if (is_null($new_type) && is_null($new_value) && is_null($new_lock_status)) {
			continue;
		}

		// Actually set the value
		if (isset($_POST[$prefix.'_link_value'][$link['linkid']])) {
			$new_value = $_POST[$prefix.'_link_value'][$link['linkid']];
		}
		// Cleanup any html
		$new_value = (is_null($new_value)) ? NULL : htmlentities($new_value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

		// Change LINK 1 or 2 to a NOTICE link
		$link_to_notice = (($link['link_type'] == SQ_LINK_TYPE_1|SQ_LINK_TYPE_2) && $new_type == SQ_LINK_NOTICE);
		$notice_to_link = (($new_type == SQ_LINK_TYPE_1|SQ_LINK_TYPE_2) && $link['link_type'] == SQ_LINK_NOTICE);
		if (!is_null($new_type) && ($link_to_notice || $notice_to_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			if (isset($link['majorid']) && !empty($link['majorid'])) {
				$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);
				if (!is_null($major)) {
					$value = (isset($link['value'])) ? $link['value'] : '';
					// Create the new link
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($major, $owner, $new_type, $new_value)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						continue;
					}
					// Delete the original link
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						continue;
					}
					// All done commit changes and leave nice
					$done_something = TRUE;
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					continue;
				}
			}
			// change link type for linked children $link_to_notice || $notice_to_link
			if (isset($link['minorid']) && !empty($link['minorid'])) {
				$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
				if (!is_null($minor)) {
					$value = (isset($link['value'])) ? $link['value'] : '';
					// Create the new link
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($owner, $minor, $new_type, $new_value)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						continue;
					}
					// Delete the original link
					if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						continue;
					}
					// All done commit changes and leave nice
					$done_something = TRUE;
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					continue;
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			continue;
		}

		// Change from LINK 1 to a LINK 2 (or changing a value/lock status only)
		if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], $new_type, $new_value, NULL, $new_lock_status)) {
			continue;
		}
		$done_something = TRUE;
	}//end foreach

	// Refresh the Asset Map to show linking changes
	if ($done_something) {
		$o->addOnLoad('if (window.top.frames["sq_sidenav"] && window.top.frames["sq_sidenav"].reload_assets) window.top.frames["sq_sidenav"].reload_assets("'.addslashes($owner->id).'");');
	}

	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if (!is_null($cron_mgr)) {

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_lineage');

		if (!empty($_POST[$prefix.'_remove_fl'])) {
			$fl_jobs = Cron_Job_Future_Lineage::getActiveJobs($owner,$display_reverse);
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

		$fl = new Cron_Job_Future_Lineage();
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

	$fl_create = new Cron_Job_Future_Lineage();
	$edit_fns_create = $fl_create->getEditFns();
	$fl_create->setAssetInLink($owner, $display_reverse ? 'major' : 'minor');


	// we process the create new link
	// if all the parameters are set properly we add to the cron manager the new job
	if (!$display_reverse && $edit_fns_create->processInLineBackendCreate($fl_create, $o, $prefix.'_create')) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($cron_mgr->addJob($fl_create, $GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$done_something = TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}

	// we process the deletion of links
	// if all the parameters for the cron are set properly
	if (!$display_reverse && $edit_fns_create->processInLineBackendDelete($fl_create, $o, $prefix.'_delete')) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($cron_mgr->addJob($fl_create, $GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$done_something = TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}


	// we process the create new link for linked children
	if ($display_reverse && $edit_fns_create->processInLineBackendCreateReverse($fl_create, $o, $prefix.'_create_reverse')) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($cron_mgr->addJob($fl_create, $GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$done_something = TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}

	// we process the deletion of links for linked children
	if ($display_reverse && $edit_fns_create->processInLineBackendDeleteReverse($fl_create, $o, $prefix.'_delete_reverse')) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($cron_mgr->addJob($fl_create, $GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$done_something = TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}


	return $done_something;

}//end processLinking()


/**
* Prints each line in the linking screen
*
* @param object	&$owner				the asset whose interface we are painting
* @param array	$parent_data		Parent Information (for asset lineage)
* @param array	$link_tree			The link tree information
* @param string	$majorid			The assetid of the parent
* @param array	$link_data			The link information
* @param array	&$editing_linkids	The array of allowable edit links
* @param boolean $display_reverse   Print linked children or linked parents
*
* @return string
* @access private
*/
function _printLine(&$owner, $majorid, $link_data, $parent_data=Array(), $link_tree=Array(), &$editing_linkids=Array(), $display_reverse = FALSE)
{
	$write_access = $owner->writeAccess('links');

	$editable_link_types = Array(
							SQ_LINK_TYPE_1	=> link_type_name(SQ_LINK_TYPE_1),
							SQ_LINK_TYPE_2	=> link_type_name(SQ_LINK_TYPE_2),
							SQ_LINK_NOTICE	=> link_type_name(SQ_LINK_NOTICE),
						   );

	// prefix for all the form vars
	$prefix = $owner->getPrefix();
			$link_type = $link_data['link_type'];
			$linkid = $link_data['linkid'];
			$link_value = $link_data['value'];
			$link_locking = array_get_index($link_data, 'locked', '0');

			// work out if the user can change the link types
			$can_edit = ($write_access && in_array($link_type, array_keys($editable_link_types)) && !in_array($linkid, $editing_linkids));

			$asset_lineage = '';
			$asset_lineage_ids = Array();
			foreach ($link_tree as $tree_data) {
				$asset_lineage_ids[] = $tree_data['assetid'];
				if ($tree_data['assetid'] == $majorid) continue;
				$asset_lineage .= htmlspecialchars($tree_data['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).' &gt; ';
			}

			if(!$display_reverse)
				$asset_lineage_ids[] = $owner->id;

			if (!empty($parent_data)) {
				$asset_lineage .= '<b>'.htmlspecialchars($parent_data['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).'</b>';
			}
			if (isset($output_lineages[$asset_lineage]) && !$can_edit) {
				return '';
			}

			ob_start();
				?>
				<tr>
					<td class="sq-backend-table-cell" style="text-align: center;">
						<a href="#">
						<?php echo sq_print_icon(sq_web_path('lib').'/web/images/icons/asset_locator.png', 16, 16, translate('show_in_asset_map'), translate('show_in_asset_map'),' style="border: 0; cursor: pointer"  onclick="'.asset_locator_onclick($asset_lineage_ids, TRUE).'"');
						?>
						</a>
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
					<td style="text-align: center;">
						<?php
						if (!$can_edit) {
							echo $link_value;
						} else {
							text_box($prefix.'_link_value['.$linkid.']', html_entity_decode($link_value, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET), 15, FALSE);
						}
						?>
					</td>
					<td style="text-align: center;">
						<?php
						if ($link_type <= 2) {
							$lock_status = ($link_locking == '1') ? 'tick' : 'cross';
							$lock_status_desc = ($link_locking == '1') ? 'locked' : 'unlocked';
							if ($can_edit) {
								check_box($prefix.'_link_locked['.$linkid.']', '1', (($link_locking == '1') ? TRUE : FALSE));
							} else {
								echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$lock_status.'.gif" alt="'.translate($lock_status_desc).'" style="border: 0;" />';
							}//end if
						}//end if
						?>
					</td>
					<td><?php echo  $asset_lineage; ?></td>
				</tr>
				<?php
				$output = ob_get_contents();
			ob_end_clean();

	return $output;

}//end _printLine()


?>
