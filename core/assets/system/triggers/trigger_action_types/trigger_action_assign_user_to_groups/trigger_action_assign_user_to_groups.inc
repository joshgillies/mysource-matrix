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
*/


/**
* Trigger Action Assign User To Groups
*
* @author  Ash Karelia <akarelia@squiz.net>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Assign_User_To_Groups extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check settings, set defaults if necessary
		$link_visibility              = array_get_index($settings, 'link_visibility', 'physical');
		$conditional_options_selected = array_get_index($settings, 'conditional_options_selected', 'any');
		$link_type_to_use             = array_get_index($settings, 'link_type_to_use', 1);
		$link_value_to_use            = array_get_index($settings, 'link_value_to_use', '');
		$link_value_to_ignore         = array_get_index($settings, 'link_value_to_ignore', '');
		$link_type_to_ignore          = array_get_index($settings, 'link_type_to_ignore', Array());
		$user_group_root_node         = array_get_index($settings, 'root_node_for_groups', 0);
		$user_groups_to_join          = array_get_index($settings, 'user_groups_to_join', Array());
		$create_user_group            = array_get_index($settings, 'create_user_group', FALSE);
		$leave_joined_user_group      = array_get_index($settings, 'leave_joined_user_group', FALSE);
		$conditional_values           = array_get_index($settings, 'conditional_values', Array());
		$user_group_names             = array_get_index($settings, 'user_group_names', Array());

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		foreach ($user_groups_to_join as $index => $info) {
			if ($info['assetid'] !== 0) {
				$user_groups_to_join[$index] = $info['assetid'];
			} else {
				unset($user_groups_to_join[$index]);
			}
		}

		// check  to see if there  are any conditional  logic set up to join  the user groups
		// but first this we need to make sure there is  root node selected and that the user
		// groups to join is configured or else there is not point  evaluating the conditions
		$links_created_in_this_run = Array();

		if (!empty($user_group_names)) {
			$child_user_groups = $am->getChildren($user_group_root_node['assetid'], 'user_group', FALSE);
			$to_create_new = $children_user_group_names = Array();
			// user_group_names may or may not have global keyword in them
			// so lets find and replace the global keywords if we are passed any
			foreach ($user_group_names as $idx => $grp_name) {
				if (stripos($grp_name, '%globals_') !== FALSE) {
					replace_global_keywords($grp_name);
					if (is_string($grp_name) && stripos($grp_name, ',') !== FALSE) {
						// unset the replaced global keyword as there is no point
						// in keeping the comma separated value now
						unset($user_group_names[$idx]);
						foreach (explode(',', $grp_name) as $temp_grp) {
							array_push($user_group_names, $temp_grp);
						}
					} else if (is_array($grp_name)) {
						unset($user_group_names[$idx]);
						foreach ($grp_name as $temp_grp) {
							array_push($user_group_names, $temp_grp);
						}
					} else {
						$user_group_names[$idx] = $grp_name;
					}
				}
			}

			foreach ($child_user_groups as $child_id => $info) {
				$child_user_group = $am->getAsset($child_id);
				$children_user_group_names[$child_user_group->name] = $child_id;
				unset($child_user_group);
			}

			foreach ($user_group_names as $grp_name) {
				if (!array_key_exists($grp_name, $children_user_group_names)) {
					$to_create_new[] = $grp_name;
				}
			}

			// the conditions have passed join the user groups that we are meant to
			// be careful here are we might have been asked to create new user groups too
			// so if there any we may need to create then let's do that first
			if ($create_user_group && !empty($to_create_new)) {
				// the new link
				$link = Array(
							'asset'			=> $am->getAsset($user_group_root_node['assetid']),
							'link_type'		=> SQ_LINK_TYPE_1,
							'value'			=> '',
							'sort_order'	=> NULL,
							'is_dependant'	=> FALSE,
							'is_exclusive'	=> FALSE
						);

				$type_code = 'user_group';
				foreach ($to_create_new as $new_usr_grp_names) {
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$new_asset = new $type_code();
					$new_asset->setAttrValue('name', $new_usr_grp_names);
					$linkid = $new_asset->create($link);
					if (!$linkid) {
						// asset might not have been created
						// spit out the error and fail the trigger
						return FALSE;
					}//end if
					unset($new_asset);
				}//end foreach
			}//end if
		}

		if ((isset($user_group_root_node['assetid']) && $user_group_root_node['assetid'] != 0) && !empty($user_group_names)) {
			// this bit is a bit complex. What we need to do here is as follows:
			//		- Get al the user groups (level 1 only) under the root node selected)
			//		- for each user group get all the keyword replacements that are configured
			//		- once we have the keyword replacements we need to compare against the expected value
			//		- depending on 'All' or 'Any' Configuration we need to set the logic match as TRUE or FALSE
			//		- once the match is confirmed (case sensitive) go ahead and add the user to the user groups configured
			//				(also for ths step see if we are asked to create any new user groups that doesn't exist currently))
			$child_user_groups = $am->getChildren($user_group_root_node['assetid'], 'user_group', FALSE);
			$user_group_names_id = $children_user_group_names = Array();
			$mega_match = FALSE;

			if (!empty($child_user_groups)) {
				foreach ($child_user_groups as $child_id => $info) {
					$grp_match = FALSE;
					$child_user_group = $am->getAsset($child_id);

					if (!empty($conditional_values)) {
						foreach ($conditional_values as $idx => $keyword_info) {
							// replace these keywords against he current user group asset we are dealing with in the foreach loop
							$keyword_name_replacement  = $child_user_group->getKeywordReplacement(trim($keyword_info['name'], ' %'));
							// evaluate the asset based keywords for VALUE field against the current asset that's firing the trigger
							$keyword_value_replacement = $state['asset']->getKeywordReplacement(trim($keyword_info['value'], ' %'));

							// if the text wasn't keyword is replaced and if Matrix has just added
							// the percentage signs to our text then avoid using them
							if ($keyword_name_replacement != '%'.$keyword_info['name'].'%') {
								$keyword_info['name'] = $keyword_name_replacement;
							}

							// do the same with value part
							if ($keyword_value_replacement != '%'.$keyword_info['value'].'%') {
								$keyword_info['value'] = $keyword_value_replacement;
							}

							replace_global_keywords($keyword_info['name']);
							replace_global_keywords($keyword_info['value']);

							if (!is_null($keyword_info['value']) && $keyword_info['value'] === $keyword_info['name']) {
								// we have found a match
								$grp_match = TRUE;
								// see if we are matching all the conditions or just care about one
								if ($conditional_options_selected === 'any') {
									break;
								}
							} else {
								$grp_match = FALSE;
								if ($conditional_options_selected === 'all') {
									break;
								}
							}
						}//end foreach

						if ($grp_match === FALSE) {
							unset($child_user_groups[$child_id]);
							if (($key = array_search($child_user_group->name, $user_group_names)) !== false) {
								unset($user_group_names[$key]);
							}
						}
					} else {
						// there are no conditions set so we are theoritically passing the conditions
						$mega_match = TRUE;
						break;
					}
				}
			} else if (empty($child_user_groups) && empty($conditional_values) && !empty($user_group_names)) {
				// if there are no conditions set and if there are no user group
				// assets under the configed root node then assume there is a match
				$mega_match = TRUE;
			}

			if ($mega_match === TRUE || !empty($child_user_groups)) {
				foreach ($child_user_groups as $child_id => $info) {
					$child_user_group = $am->getAsset($child_id);
					$children_user_group_names[$child_user_group->name] = $child_id;
					unset($child_user_group);
				}

				foreach ($user_group_names as $grp_name) {
					if (array_key_exists($grp_name, $children_user_group_names) || $mega_match === TRUE) {
						$user_groups_to_join[] = $children_user_group_names[$grp_name];
					}
				}
			}//end if
		}//end if

		// we have merged everything into one array so the dynamic and the hardcoded are all in the same array
		// now we link them either as either physical link or session based linking to the user groups but first
		// we will eliminate any duplicate entries in the array :)
		$user_groups_to_join = array_unique($user_groups_to_join);

		if (!empty($user_groups_to_join)) {
			// it is configured to physically link the users
			if ($link_visibility === 'physical') {
				// if we already have a link then don't worry about recreating it
				$old_links = $am->getLinks($state['assetid'], SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
				foreach ($old_links as $old_link) {
					foreach ($user_groups_to_join as $index => $groupid) {
						if ($old_link['majorid'] == $groupid) {
							if ($old_link['link_type'] == $link_type_to_use && $old_link['value'] == $link_value_to_use) {
								$links_created_in_this_run[] = $old_link['linkid'];
							} else {
								// so we have a link to the parent we want to link to but it isn't the type or
								// doesn't have the link value which matches to us. We can't leave it hanging
								// so what we gonna do is update the link to be of configured type and  value
								$linkid = $am->updateLink($old_link['linkid'], $link_type_to_use, $link_value_to_use);
								$links_created_in_this_run[] = $old_link['linkid'];
							}
							unset($user_groups_to_join[$index]);
						}
					}
				}

				// if we are asked to create a physical link then lets create one now
				foreach ($user_groups_to_join as $groupid) {
					if ($groupid == 0) continue;
					$major = $am->getAsset($groupid);
					if (!is_null($major)) {
						$linkid = $am->createAssetLink($major, $state['asset'], $link_type_to_use, $link_value_to_use);
						if (!$linkid) return FALSE;
						$links_created_in_this_run[] = $linkid;
					}
					$major->updateLookups();
					unset($major);
				}//end foreach

				// update lookups for minor asset and all its children
				// update lookup only for the major asset (no need to update children)
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array('assetids' => Array($state['asset']->id));
				$hh->freestyleHipo('hipo_job_update_lookups', $vars);
			}//end foreach

			// also add the user groups to the session for the current user to refresh the permission without
			// having the user to logout and log back in to refresh the permission cachign done by Matrix
			foreach ($user_groups_to_join as $groupid) {
				if ($groupid == 0) continue;
				
				$groupids = $GLOBALS['SQ_SYSTEM']->am->getParents($groupid, 'user_group', FALSE);
				$groupids[$groupid] = 'user_group';
				foreach ($groupids as $grpid => $type_code) {
					if (!isset($_SESSION['sq_effective_user_groups'][$state['asset']->id]) || !in_array($grpid, $_SESSION['sq_effective_user_groups'][$state['asset']->id])) {
						$_SESSION['sq_effective_user_groups'][$state['asset']->id][] = $grpid;
						$_SESSION['sq_session_user_groups'][$state['asset']->id][] = $grpid;
					}//end if
				}//end if
			}//end freach
		}//end if

		// now delete all the link type/value that have been configured on the trigger action
		if ($leave_joined_user_group == TRUE) {
			$link_value_to_ignore = empty($link_value_to_ignore) ? NULL : $link_value_to_ignore;
			$link_type = 0;
			foreach ($link_type_to_ignore as $link_type_value) {
				$link_type += $link_type_value;
			}

			$all_parent_links = $am->getLinks($state['asset']->id, $link_type, 'user_group', TRUE, 'minor', $link_value_to_ignore);

			if (!empty($all_parent_links)) {
				$deleted_parent_links = Array();

				foreach ($all_parent_links as $parent_link) {
					// if the link isn't falling under our configured root node then don't delete
					if (!$GLOBALS['SQ_SYSTEM']->am->isUnderRootNodes($parent_link['majorid'], Array($user_group_root_node['assetid']))) continue;

					// if the link is created by this trigger action in current transaction then it's a bit stupid
					// to delete it whithout doin much with it. So in that case skip deleting the links and b happy
					if (in_array($parent_link['linkid'], $links_created_in_this_run) === TRUE) continue;
					$deleted_parent_links[] = $parent_link['majorid'];
					$res = $am->deleteAssetLink($parent_link['linkid']);
					if (!$res) return FALSE;
				}//end foreach

				if (isset($_SESSION['sq_effective_user_groups'][$state['asset']->id])) {
					foreach ($_SESSION['sq_effective_user_groups'][$state['asset']->id] as $index => $group_id) {
						if (in_array($group_id, $deleted_parent_links)) {
							unset($_SESSION['sq_effective_user_groups'][$state['asset']->id][$index]);
						}//end if
					}//end foreach
				}

				if (isset($_SESSION['sq_session_user_groups'][$state['asset']->id])) {
					foreach ($_SESSION['sq_session_user_groups'][$state['asset']->id] as $index => $group_id) {
						if (in_array($group_id, $deleted_parent_links)) {
							unset($_SESSION['sq_session_user_groups'][$state['asset']->id][$index]);
						}//end if
					}//end foreach
				}
			}//end if
		}//end if

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check settings, set defaults if necessary
		$link_visibility              = array_get_index($settings, 'link_visibility', 'physical');
		$conditional_options_selected = array_get_index($settings, 'conditional_options_selected', 'any');
		$link_type_to_use             = array_get_index($settings, 'link_type_to_use', 1);
		$link_value_to_use            = array_get_index($settings, 'link_value_to_use', '');
		$link_value_to_ignore         = array_get_index($settings, 'link_value_to_ignore', '');
		$link_type_to_ignore          = array_get_index($settings, 'link_type_to_ignore', Array());
		$user_group_root_node         = array_get_index($settings, 'root_node_for_groups', 0);
		$current_groups               = array_get_index($settings, 'user_groups_to_join', Array());
		$create_user_group            = array_get_index($settings, 'create_user_group', FALSE);
		$leave_joined_user_group      = array_get_index($settings, 'leave_joined_user_group', FALSE);
		$conditional_values           = array_get_index($settings, 'conditional_values', Array());
		$user_group_names             = array_get_index($settings, 'user_group_names', Array());

		$asset_types_allowed = Array('user_group' => 'D');
		ob_start();
			$linking_options = Array('session' => translate('for the current session'), 'physical' => translate('as a physical link'));
			if ($write_access) {
				combo_box($prefix.'[link_visibility]', $linking_options, FALSE, $link_visibility, 0, 'onChange="toggleLinkTypeValue(this); return false;"');
			} else {
				combo_box($prefix.'[link_visibility]', $linking_options, FALSE, $link_visibility, 0, 'disabled="disabled"');
			}

			$basic_part_1= ob_get_contents();
		ob_end_clean();

		ob_start();
			$current_user_groups = Array();
			foreach ($current_groups as $groups) {
				if (isset($groups['assetid']) && !empty($groups['assetid'])) $current_user_groups[] = $groups['assetid'];
			}
			if ($write_access) {
				multiple_asset_finder($prefix.'[user_groups_to_join]', $current_user_groups, $asset_types_allowed);
			} else {
				if (!empty($current_user_groups)) {
					echo '<ul>';
					foreach ($current_user_groups as $group) {
						echo '<li>'.get_asset_tag_line($group).'</li>';
					}
					echo '</ul>';
				} else {
					echo '<strong><em>No User Groups Selected</em></strong>';
				}
			}

			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			if ($write_access) {
				if ($user_group_root_node != 0 && isset($user_group_root_node['assetid'])) $user_group_root_node = $user_group_root_node['assetid'];
				asset_finder($prefix.'[root_node_for_groups]', $user_group_root_node, Array('asset' => 'D'));
			} else {
				if (isset($user_group_root_node['assetid']) && $user_group_root_node['assetid'] != 0) {
					echo '<strong><em>'.get_asset_tag_line($user_group_root_node['assetid']).'</em></strong>';
				} else {
					echo '<strong><em>No Root Node Selected</em></strong>';
				}
			}

			$basic_part_3 = ob_get_contents();
		ob_end_clean();


		ob_start();
			$conditional_options = Array ('any' => translate('Any'), 'all' => translate('All'));
			if ($write_access) {
				combo_box($prefix.'[conditional_options_selected]', $conditional_options, FALSE, $conditional_options_selected);
			} else {
				combo_box($prefix.'[conditional_options_selected]', $conditional_options, FALSE, $conditional_options_selected, 0, 'disabled="disabled"');
			}

			$basic_part_4 = ob_get_contents();
		ob_end_clean();


		ob_start();
			// get delete icon, with new lines replaced
			sq_print_icon(sq_web_path('data').'/asset_types/bodycopy/images/icons/delete.png', 16, 16, translate('Delete'), translate('Delete'), ' style="border:none"');

		$delete_icon = ob_get_clean();
		$delete_icon = str_replace("\n", ' ', $delete_icon);


		ob_start();
		// TODO
			if ($write_access) {
				?>
					<div id="option-list-<?php echo $prefix; ?>" class="sq-option-list">
						<?php
							if (empty($conditional_values)) {
								// max() won't work on an empty array
								$last_key = -1;
							} else {
								$last_key = max(array_keys($conditional_values));
							}
						?>
						<script type="text/javascript">
								trigger_action_join_user_group = {
									nextIndex: [],

									/**
									 * Adds a new row.
									 */
									addGetRow: function(prefix) {
										node = document.getElementById('table_' + prefix + '_conditional_values_body');
										index = this.nextIndex['table_' + prefix];

										// create a new row
										tr = document.createElement('tr');
										tr.id = 'table_' + prefix + '_conditional_values_row_' + index;

										// build the cells
										td1 = document.createElement('td');
										td1.innerHTML = '<?php text_box("' + prefix + '[conditional_values][' + index + '][name]", '', '', '', FALSE, 'class="sq-full-cell-width"') ?>';
										td2 = document.createElement('td');
										td2.innerHTML = '<?php text_box("' + prefix + '[conditional_values][' + index + '][value]", '', '', '', FALSE, 'class="sq-full-cell-width"') ?>'
										td3 = document.createElement('td');
										td3.innerHTML = '<a href="" onclick="trigger_action_join_user_group.removeGetRow(\'table_' + prefix +'\', \'' + index + '\'); return false;"><?php echo $delete_icon; ?></a>';
										td3.align= 'center';

										// Now put them all together
										tr.appendChild(td1);
										tr.appendChild(td2);
										tr.appendChild(td3);
										node.appendChild(tr);

										// Go to next index
										this.nextIndex['table_' + prefix]++;

										// Update display of "no variable" status
										this.updateNoVarStatus('table_' + prefix);
									},

									/*
									 * Removes a row.
									 */
									removeGetRow: function(prefix, index) {
										node = document.getElementById(prefix + '_conditional_values_row_' + index);
										node.parentNode.removeChild(node);
										this.updateNoVarStatus(prefix);
									},

									/*
									 * Updates display of the "no variable" body.
									 */
									updateNoVarStatus: function(prefix) {
										node = document.getElementById(prefix + '_conditional_values_body');
										var noVarBody = document.getElementById(prefix + '_conditional_values_none');
										noVarBody.style.display = (node.getElementsByTagName('tr').length > 0) ? 'none' : 'table-row-group';
									}
								};

							// Set the first new index for newly created rows
							trigger_action_join_user_group.nextIndex['table_<?php echo $prefix; ?>'] = <?php echo ($last_key + 1); ?>;
						</script>
						<table class="sq-backend-table" style="margin: 5px 0;">
							<colgroup>
								<col width="47%" />
								<col width="47%" />
								<col />
							</colgroup>
							<thead>
								<tr>
									<th>User Group Keyword Replacement</th>
									<th>Set Value or Keyword Replacement</th>
									<th><span style="white-space: nowrap"><?php echo translate('Delete?') ?></span></th>

								</tr>
							</thead>
							<tbody id="table_<?php echo $prefix ?>_conditional_values_none" style="display: <?php echo empty($conditional_values) ? 'table-row-group' : 'none' ?>">
								<tr>
									<td colspan="2"><em><?php echo translate('No value matching conditions have been set. Value matching will not be used.'); ?></em></td>

								</tr>
							</tbody>
							<tbody id="table_<?php echo $prefix ?>_conditional_values_body">
							<?php
							foreach ($conditional_values as $key => $var_info) {
								?>
								<tr id="table_<?php echo $prefix; ?>_conditional_values_row_<?php echo $key; ?>">
									<td><?php text_box($prefix.'[conditional_values]['.$key.'][name]', $var_info['name'], '', '', FALSE, 'class="sq-full-cell-width"') ?></td>
									<td><?php text_box($prefix.'[conditional_values]['.$key.'][value]', $var_info['value'], '', '', FALSE, 'class="sq-full-cell-width"') ?></td>
									<td align="center"><a href="" onclick="trigger_action_join_user_group.removeGetRow('table_<?php echo $prefix; ?>','<?php echo $key; ?>'); return false;"><?php echo $delete_icon; ?></a></td>
								</tr>
								<?php
							}
							?>
							</tbody>
						</table>
					</div>
						<input type="button" name="table_<?php echo $prefix ?>_conditional_values_new_row" id="table_<?php echo $prefix ?>_conditional_values_new_row" value="Add Condition" onclick="trigger_action_join_user_group.addGetRow('<?php echo $prefix ?>'); return false;"/>
					<?php
				} else {
					?>
						<table class="sq-backend-table">
							<colgroup>
								<col width="50%" />
								<col width="50%" />
								<col />
							</colgroup>
							<thead>
								<tr>
									<th>User Group Keyword Replacement</th>
									<th>Set Value or Keyword Replacement</th>
								</tr>
							</thead>
							<?php
								if (empty($conditional_values)) {
								?>
								<tbody id="table_<?php echo $prefix ?>_conditional_values_none" style="display: <?php echo empty($conditional_values) ? 'table-row-group' : 'none' ?>">
									<tr>
										<td colspan="2"><em><?php echo translate('No value matching conditions have been set. Value matching will not be used.'); ?></em></td>

									</tr>
								</tbody>
								<?php
								} else {
									?>
									<tbody id="table_<?php echo $prefix ?>_conditional_values_body">
									<?php
									foreach ($conditional_values as $key => $var_info) {
										?>
										<tr id="table_<?php echo $prefix; ?>_conditional_values_row_<?php echo $key; ?>">
											<td><?php text_box($prefix.'[conditional_values]['.$key.'][name]', $var_info['name'], '', '', FALSE, 'class="sq-full-cell-width" disabled="disabled"'); ?></td>
											<td><?php text_box($prefix.'[conditional_values]['.$key.'][value]', $var_info['value'], '', '', FALSE, 'class="sq-full-cell-width" disabled="disabled"'); ?></td>
										</tr>
										<?php
									}
									?>
									</tbody>
									<?php
								}
							?>
						</table>
					<?php
				}

			$basic_part_5 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				check_box($prefix.'[create_user_group]', '1', $create_user_group, '', 'id="'.$prefix.'_create_user_group"');
				label(translate('Create user groups not found by name and join user into it'), $prefix.'_create_user_group');
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.(empty($create_user_group) ? 'cross' : 'tick').'.png" />'.translate(' Create user groups not found by name and join user into it');
			}

			$basic_part_6 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				text_box($prefix.'[user_group_names]', implode(',', $user_group_names), '100', '', FALSE);
			} else {
				text_box($prefix.'[user_group_names]', implode(',', $user_group_names), '100', '', FALSE, 'disabled="disabled"');
			}

			$basic_part_7 = ob_get_contents();
		ob_end_clean();


		$link_type_list = get_link_type_names();
		$disabled = ($link_visibility == 'session') ? 'disabled="disabled"' : '';
		unset($link_type_list[8]);
		ob_start();
			if ($write_access) {
				combo_box($prefix.'[link_type_to_use]', $link_type_list, FALSE, $link_type_to_use, 0, $disabled);
			} else {
				combo_box($prefix.'[link_type_to_use]', $link_type_list, FALSE, $link_type_to_use, 0, 'disabled="disabled"');
			}

			$basic_part_8 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				text_box($prefix.'[link_value_to_use]', $link_value_to_use, '', '', FALSE, $disabled);
				?>
				<script type="text/javascript">
					function toggleLinkTypeValue(e) {
						var prefix = '<?php echo $prefix; ?>';
						var link_type_combo  = document.getElementById(prefix + '[link_type_to_use]');
						var link_value_combo = document.getElementById(prefix + '[link_value_to_use]');
						if (e.value == 'session') {
							link_type_combo.disabled  = true;
							link_value_combo.disabled = true;
						} else {
							link_type_combo.disabled  = false;
							link_value_combo.disabled = false;
						}
					}
				</script>
				<?php
			} else {
				text_box($prefix.'[link_value_to_use]', $link_value_to_use, '', '', FALSE, 'disabled="disabled"');
			}

			$basic_part_9 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				check_box($prefix.'[leave_joined_user_group]', '1', $leave_joined_user_group);
				label(translate('Leave all other joined user groups linked using '), $prefix.'[leave_joined_user_group]');
			} else {
				echo '<img src="'.sq_web_path('lib').'/web/images/'.(empty($leave_joined_user_group) ? 'cross' : 'tick').'.png" />'.translate(' Leave all other joined user groups linked using ');
			}

			$basic_part_10 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				combo_box($prefix.'[link_type_to_ignore]', $link_type_list, TRUE, $link_type_to_ignore);
			} else {
				combo_box($prefix.'[link_type_to_ignore]', $link_type_list, TRUE, $link_type_to_ignore, 0, 'disabled="disabled"');
			}

			$basic_part_11 = ob_get_contents();
		ob_end_clean();


		ob_start();
			if ($write_access) {
				text_box($prefix.'[link_value_to_ignore]', $link_value_to_ignore);
			} else {
				text_box($prefix.'[link_value_to_ignore]', $link_value_to_ignore, '', '', FALSE, 'disabled="disabled"');
			}

			$basic_part_12 = ob_get_contents();
		ob_end_clean();


		$contents = '<p class="sq-backend-section-subheading">'.translate('User Group Linking Settings').'</p>'.

					'<table class="sq-backend-table with-bottom-margin">'.

						'<tr>'.
							'<th width="190px">'.translate('Join Method').'</th>'.
							'<td><span class="sq-inline-fields-wrapper">'.translate('Join current user to specified user groups '). $basic_part_1.'</span></td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('Link Type').'</th>'.
							'<td><span class="sq-inline-fields-wrapper">'.sprintf(translate(' Use a %1$s link and link value of %2$s'), $basic_part_8, $basic_part_9).'</span></td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('Fixed User Group To Join').'</th>'.
							'<td>'.$basic_part_2.'</td>'.
						'</tr>'.

					'</table>'.
					
					'<p class="sq-backend-section-subheading">'.translate('User Groups To Join By Name').'</p>'.

					'<table class="sq-backend-table">'.

						'<tr>'.
							'<th width="190px">'.translate('Root Node To Join Groups Under').'</th>'.
							'<td>'.$basic_part_3.
								'<br /><div class="sq-backend-note">'.
									translate('Only user groups under this root node will be joined by names.').
								'</div>'.
							'</td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('User Groups To Join By Asset Name').'</th>'.
							'<td>'.$basic_part_7.
								'<br /><div class="sq-backend-note">'.
									translate('Enter user group names to join. Multiple user group names can be set as comma separated values and are case sensitive. Globals keywords are supported in this field. If left empty, no user groups will be joined by name.').
								'</div>'.
							'</td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('Create And Join Missing User Groups?').'</th>'.
							'<td>'.$basic_part_6.
								'<br /><div class="sq-backend-note">'.
									translate('Enabling this option will make the trigger create user groups (Under Construction) under the specified root node above for any user groups not found by name.').
								'</div>'.
							'</td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('Join Conditions').'</th>'.
							'<td><span class="sq-inline-fields-wrapper">'.sprintf(translate('Only join user groups that match on %1$s of the following conditions:'), $basic_part_4).'</span>'.
								$basic_part_5.
							'</td>'.
						'</tr>'.

						'<tr>'.
							'<th>'.translate('Leave Groups Settings').'</th>'.
							'<td><span class="sq-inline-fields-wrapper">'.sprintf(translate('%1$s %2$s link and link value %3$s'), $basic_part_10, $basic_part_11, $basic_part_12).'</span>'.
								'<br /><div class="sq-backend-note">'.
									translate('This only applies to user groups linked under the root node set above.').
								'</div>'.
							'</td>'.
						'</tr>'.

					'</table>';

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['root_node_for_groups'] = array_get_index($request_data, 'root_node_for_groups', 0);
		$settings['user_groups_to_join'] = array_get_index($request_data, 'user_groups_to_join', Array());

		if (empty($settings['user_groups_to_join']) && $settings['root_node_for_groups'] === 0) {
			return translate('Neither the \'User Groups to Join\' or the \'Root Node\' setting to look for user groups under is specified');
		}

		$settings['link_visibility']              = array_get_index($request_data, 'link_visibility', 'physical');
		$settings['conditional_options_selected'] = array_get_index($request_data, 'conditional_options_selected', 'any');
		$settings['create_user_group']            = array_get_index($request_data, 'create_user_group', FALSE);;
		$settings['link_type_to_use']             = array_get_index($request_data, 'link_type_to_use', 1);
		$settings['link_value_to_use']            = array_get_index($request_data, 'link_value_to_use', '');
		$settings['leave_joined_user_group']      = array_get_index($request_data, 'leave_joined_user_group', FALSE);
		$settings['link_value_to_ignore']         = array_get_index($request_data, 'link_value_to_ignore', '');
		$settings['link_type_to_ignore']          = array_get_index($request_data, 'link_type_to_ignore', Array());
		$settings['conditional_values']           = array_get_index($request_data, 'conditional_values', Array());
		foreach ($settings['conditional_values'] as $index => $info) {
			if (trim($info['name']) === '') unset($settings['conditional_values'][$index]);
		}

		$user_group_names_string                  = array_get_index($request_data, 'user_group_names', '');
		$user_group_names_array = explode(',' , $user_group_names_string);
		foreach ($user_group_names_array as $grp_name) {
			if ($grp_name != '') $settings['user_group_names'][] = trim($grp_name);
		}

		return FALSE;

	}//end processInterface()


}//end class

?>
