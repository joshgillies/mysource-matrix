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
* $Id: details.inc,v 1.52 2012/10/30 04:17:15 akarelia Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Details Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.52 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing details
*
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o			the outputter class
* @param object	&$ei		the edit interface
* @param array	$keywords	the list of keywords
*
* @return boolean
* @access public
*/
function paintDetails(&$owner, &$o, &$ei, $keywords=NULL)
{
	// no static details screen fields relevant to an uncreated asset
	if (empty($owner->id)) return;

	$root = $ei->findScreen('static_screen_details');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();
	$print_commit_button = FALSE;

	$display_status_sections = TRUE;
	$parent_asset_href = '';
	// check if the asset is linked to its parent exclusively and is dependent
	$parent_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_SC_LINK_ALL, '', TRUE, 'minor', NULL, TRUE, TRUE);
	if (!empty($parent_asset_link)) {
		$display_status_sections = FALSE;
		$parent_asset_href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($parent_asset_link[0]['majorid'] => 'details'));
	}

	foreach ($root->section as $section) {

		// only do this if we are not painting a layout
		if (is_null($keywords)) {
			// if we are in limbo and this section is not to be printed in limbo - dont print it
			if (SQ_IN_LIMBO && (!isset($section->attributes()->limbo_access) || (int)$section->attributes()->limbo_access == 0)) {
				continue;
			}
		}

		$section_keyword_name = strtolower(str_replace(' ', '_', $section->attributes()->name));

		if (!$ei->_getAccess($owner, $section, $prefix)) {
			continue;
		}

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		ob_start();

		// If this is the Safe Trash section, hide the section if the asset is
		// not attached to the Safe Trash cron job
		$safe_trash_job_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'cron_job_attempt_safe_trash', TRUE, NULL, 'minor');
		$avoiding_safe_trash_section = ($section->attributes()->name == 'Safe Trash Status') && !$safe_trash_job_link;
		if (!$avoiding_safe_trash_section) {
			$o->openSection(array_get_index($section->attributes(), 'display_name', $section->attributes()->name));
		}

		foreach ($section->field as $field) {

			switch (strtolower($field->attributes()->code)) {

				case 'safe_trash' :

					if (!$avoiding_safe_trash_section) {
						$safe_trash_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
						$safe_trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($owner->id);
						$o->openRaw();
							if ($safe_trash_enabled) {
								if (empty($safe_trash_errors)) {
								?>
								<div><?php echo translate('This asset is marked to be automatically trashed when it meets Safe-Trash conditions.'); ?><br/>

								<?php echo translate('This asset now satisfies those conditions and will now be trashed at the next run of the Scheduled Jobs Manager.'); ?></div>

								<?php
								} else {
								?>
								<div class="sq-backend-warning"><strong><?php echo translate('This asset is marked to be automatically trashed when it meets Safe-Trash conditions.'); ?><br/>

								<?php echo translate('There are still Safe-Trash conditions that prevent this asset from being trashed:'); ?></strong></div>


								<ul>
								<?php
								if (!empty($safe_trash_errors['status'])) {
								?>
								<li><?php echo translate('The asset\'s status is Live or higher status. Change the status to <em>Under Construction</em> or <em>Archived</em> to satisfy this condition.'); ?></li>

								<?php
								}
								?>
								<?php
								if (!empty($safe_trash_errors['links'])) {
								?>
								<li><?php echo sprintf(translate('There are NOTICE or TYPE_3 links from another asset to this asset. See the <a href="%s">Linking screen</a> for details of all this asset\'s links.'), $owner->getBackendHref('linking')); ?></li>

								<?php
								}
								?>
								<?php
								if (!empty($safe_trash_errors['children'])) {
								?>
								<li><?php echo translate('Any of the asset\'s children is either of Live or higher status, or is NOTICE or TYPE_3 linked to another asset.'); ?></li>

								<?php
								}
								?>
								</ul></div>
								<?php
								}
							} else {
								?>
								<div><?php echo translate('This asset is marked to be automatically trashed when it meets Safe-Trash conditions.'); ?><br/>

								<?php echo translate('However, Safe-Trash is currently disabled. This asset will be trashed at the next run of the Scheduled Jobs Manager.'); ?></div>

								<?php
							}

						$o->closeRaw();
					}

				break;

				///// STATUS CHANGES - NOW AND IN THE FUTURE ////

				case 'status_current' :

					if (!empty($field->display_name)) {
						$display_name = translate('Current');

					} else {
						// If we dont have a display name node then we dont have anything to say
						$display_name = '';
					}

					$o->openField($display_name);

						ob_start();
							$need_commit = _paintDetailsCurrentStatus($owner, $o, $prefix);
							$print_commit_button = $print_commit_button || $need_commit;
						$contents = ob_get_clean();

						// If unable to save as a keyword replacement, echo it
						if (_paintDetailsRegisterKeyword($ei, $keywords, 'fields', 'status_current', $contents) === FALSE) {
							echo $contents;
						}

					$o->closeField();

				break;

				case 'status_change' :

					if (!$display_status_sections) {
						$o->openField('');
							echo translate('asset_current_status_change_from_parent',$parent_asset_href[$parent_asset_link[0]['majorid']]);
						$o->closeField();
						break;
					}

					if (!empty($field->display_name)) {
						$display_name = translate('Change?');

					} else {
						// If we dont have a display name node then we dont have anything to say
						$display_name = '';
					}


					$o->openField($display_name);

						ob_start();
							$need_commit = _paintDetailsChangeStatus($owner, $o, $prefix);
							$print_commit_button = $print_commit_button || $need_commit;
						$contents = ob_get_clean();

						// If unable to save as a keyword replacement, echo it
						if (_paintDetailsRegisterKeyword($ei, $keywords, 'fields', 'status_change', $contents) === FALSE) {
							echo $contents;
						}

					$o->closeField();

				break;

				case 'status_future_status' :
					if (!$display_status_sections) {
						$o->openField('');
							echo translate('asset_future_status_change_from_parent',$parent_asset_href[$parent_asset_link[0]['majorid']]);
						$o->closeField();
						break;
					}

					// One simple edit keyword covers TWO sections here, so
					// a little more maneuvering is required
					ob_start();
						$need_commit = _paintDetailsFutureStatusCurrent($owner, $o, $prefix);
						$print_commit_button = $print_commit_button || $need_commit;
					$current_fs_contents = ob_get_clean();

					ob_start();
						$need_commit = _paintDetailsFutureStatusNew($owner, $o, $prefix);
						$print_commit_button = $print_commit_button || $need_commit;
					$new_fs_contents = ob_get_clean();

					// If unable to save the lot as a keyword replacement...
					if (_paintDetailsRegisterKeyword($ei, $keywords, 'fields', 'status_future_status', $current_fs_contents.$new_fs_contents) === FALSE) {
						// Only show current contents if there are set FS jobs
						if (empty($current_fs_contents) === FALSE) {
							$o->openField(translate('Scheduled Future Status Changes'));

								echo $current_fs_contents;
							$o->closeField();
						}

						// Only show Add FS section if write access allowed
						if ($owner->writeAccess()) {
							$o->openField(translate('Add New Future Status'));

								echo $new_fs_contents;
							$o->closeField();
						}//end if

					}

				break;

				case 'thumbnail_change' :

					if ($owner->writeAccess('links')) {

						if (!empty($field->display_name)) {
							$display_name = translate('Change?');

						} else {
							$display_name = '';
						}

						$o->openField($display_name);

							ob_start();
								$need_commit = _paintDetailsThumbnailChange($owner, $o, $prefix);
								$print_commit_button = $print_commit_button || $need_commit;
							$contents = ob_get_clean();

							// If unable to save as a keyword replacement, echo it
							if (_paintDetailsRegisterKeyword($ei, $keywords, 'fields', 'thumbnail_change', $contents) === FALSE) {
								echo $contents;
							}

						$o->closeField();
					}

				break;

				case 'thumbnail_current' :

					if (!empty($field->display_name)) {
						$display_name = translate('Current');

					} else {
						$display_name = '';
					}

					$o->openField($display_name);

						ob_start();
							$need_commit = _paintDetailsThumbnailCurrent($owner, $o, $prefix);
							$print_commit_button = $print_commit_button || $need_commit;
						$contents = ob_get_clean();

						// If unable to save as a keyword replacement, echo it
						if (_paintDetailsRegisterKeyword($ei, $keywords, 'fields', 'thumbnail_current', $contents) === FALSE) {
							echo $contents;
						}

					$o->closeField();

				break;

			}//end switch

			$note = (string) $field->note;
			if (!empty($note)) {
				$o->note($note);
			}

		}//end for

		if (!$avoiding_safe_trash_section) {
			$o->closeSection();
		}

		$section_contents = ob_get_clean();

		if (_paintDetailsRegisterKeyword($ei, $keywords, 'sections', $section_keyword_name, $section_contents) === FALSE) {
			echo $section_contents;
		}

	}//end for

	return $print_commit_button;

}//end paintDetails()


/**
* Paints the interface for editing the status of an asset
*
* It returns the array of of statii shown in the drop down
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param string	$prefix	prefix for form vars
*
* @return array
* @access public
*/
function _paintStatusChange(&$owner, &$o, $prefix, $statii, $default_status=0)
{
	$is_bundle = $owner->type() === 'workflow_bundle';

	if (!empty($statii)) {
		$statii = array_reverse($statii, TRUE);
		$statii[0] = '';
		$statii = array_reverse($statii, TRUE);
		combo_box($prefix.'_change_status', $statii, FALSE, $default_status, 0, 'onchange="checkStatusDisplayWarning()"');

		// Test whether we have a status that may need the stream
		$has_pending_status = FALSE;
		$has_approved_status = FALSE;
		$editor_commenting = FALSE;
		foreach ($statii as $status_code => $status_text) {
			if (($status_code & SQ_SC_STATUS_PENDING) > 0) {
				$has_pending_status = TRUE;
				break;
			}

			if (($status_code & SQ_SC_STATUS_ALL_APPROVED) > 0) {
				$has_approved_status = TRUE;
				break;
			}
		}

		// Workflow streams activate to those where:
		// user has admin permission or write permission (depends on setting)
		// - one of the options is a Pending status (either a Apply for Approval,
		//   or Place Up for Review);
		// - the current status is either a "can approve" state (Safe Edit or
		//   U/C), or Live (otherwise we couldn't run Up for Review on a stream).

		if (($owner->writeAccess() === TRUE) && (($owner->status & (SQ_STATUS_LIVE | SQ_SC_STATUS_CAN_APPROVE)) > 0) && (($has_pending_status === TRUE) || ($has_approved_status === TRUE))) {
			echo '&nbsp;';
			// If it is possible that we will have a "apply for approval" in the
			// drop-down, paint the streams box and enable/disable when required.
			$stream_names = Array(
								'workflow'	=> Array(),
								'bypass'	=> Array(),
							);

			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$schemas = $wfm->getSchemas($owner->id);
			foreach ($schemas as $schemaid => $granted) {
				$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);

				// should we allow commenting
				if (!$editor_commenting && $schema->attr('allow_comment_on_request')) {
					$editor_commenting = TRUE;
				}


				// if schema requires admin access user only to choose stream and we are not admin user, then we have to skip stream selection
				$who_can_choose_stream = $schema->attr('who_can_choose_stream');
				if($who_can_choose_stream === 'admin_only' && !$owner->adminAccess()) continue;

				$stream_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($schemaid, SQ_LINK_TYPE_2, 'workflow_stream', TRUE);
				$streamids = Array();
				$default_streamid = '';
				foreach ($stream_links as $stream_link) {
					if ($stream_link['value'] !== 'default_stream') {
						$streamids[] = $stream_link['minorid'];
					}
					else {
						$default_streamid = $stream_link['minorid'];
					}
				}
				// evaluate stream selection rules and get a list of valid streams
				$valid_streams = $wfm->getValidStreams($schemaid, $owner->id);

				if ($has_approved_status === TRUE) {
					// We can bypass workflow on at least one stream
					$bypassable_streams = $wfm->getBypassableStreams($owner->id, $schemaid);
					$bypassable_streams = array_intersect($bypassable_streams, $streamids);
					$workflowable_streams = array_diff($streamids, $bypassable_streams);

					$bypass_stream_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($bypassable_streams, 'workflow_stream', TRUE, 'name');
					// evaluate stream rules and determine which rule shouldn't be shown to user
					foreach ($bypass_stream_names as $streamid => $name) {
						if(!in_array($streamid, $valid_streams))
							unset($bypass_stream_names[$streamid]);
					}
					$stream_names['bypass'] = array_merge($stream_names['bypass'], $bypass_stream_names);
				} else {
					// We cannot bypass (either none work, or metadata is not finished)
					$workflowable_streams = $streamids;
				}

				$workflow_stream_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($workflowable_streams, 'workflow_stream', TRUE, 'name');
				// evaluate stream rules and determine which rule shouldn't be shown to user
				foreach ($workflow_stream_names as $streamid => $name) {
					if(!in_array($streamid, $valid_streams))
						unset($workflow_stream_names[$streamid]);
				}


				$stream_names['workflow'] = array_merge($stream_names['workflow'], $workflow_stream_names);

			}//end foreach schema

			// Make both types of streams unique, then some streams are in
			// both columns (possible if multiple schemas applied), eliminate
			// them from the bypassable column
			$stream_names['workflow'] = array_unique($stream_names['workflow']);
			$stream_names['bypass']   = array_unique($stream_names['bypass']);
			$stream_names['bypass']   = array_diff($stream_names['bypass'], $stream_names['workflow']);

			// Now generate the options
			$options = Array(
						'workflow'	=> Array(),
						'bypass'	=> Array(),
					   );

			// should we place default stream at all?
			$place_default_stream = TRUE;
			if(!empty($default_streamid) && !in_array($default_streamid, $valid_streams)) {
				if(!empty($stream_names['bypass']) || !empty($stream_names['workflow']))
					$place_default_stream = FALSE;
			}

			// Work out which set of options the default streams should be placed
			if($place_default_stream) {
				$default_test = $wfm->testPublish($owner->id, $GLOBALS['SQ_SYSTEM']->user->id, '');
				if (($has_approved_status === TRUE) && ($default_test === TRUE)) {
					$options['bypass']   = Array('SQ_USE_DEFAULT' => 'Default Stream');
				} else {
					$options['workflow'] = Array('SQ_USE_DEFAULT' => 'Default Stream');
				}
			}

			$total_options = 1;
			foreach (array_keys($stream_names) as $stream_type) {
				foreach ($stream_names[$stream_type] as $workflow_stream_name) {
					$options[$stream_type][$workflow_stream_name] = $workflow_stream_name;
					$total_options++;
				}
			}

			// We have more than one stream, so paint out both drop-downs
			if (count($options['bypass']) + count($options['workflow']) > 1) {
				// The combo boxes are named the same, but have different IDs,
				// and one is always disabled, so only one gets sent.
				ob_start();
					combo_box($prefix.'_change_status_stream', $options['bypass'], FALSE, Array(), 0, ' disabled="disabled" id="'.$prefix.'_change_status_stream_bypass"');
				$cb1 = ob_get_clean();

				ob_start();
					combo_box($prefix.'_change_status_stream', $options['workflow'], FALSE, Array(), 0, ' disabled="disabled" id="'.$prefix.'_change_status_stream_workflow"');
				$cb2 = ob_get_clean();

				echo sprintf(translate('using workflow stream: %1$s'), $cb1.$cb2);

			} else {
				// Only one option; hide the stream field and chuck it into a hidden field
				$all_options = $options['bypass'] + $options['workflow'];
				reset($all_options);
				hidden_field($prefix.'_change_status_stream', key($all_options));
			}


			// Javascript to keep the stream lists happy. The "Apply for Approval"
			// list is displayed first, and they are switched when "Apply for Approval"
			// and "Approve" are selected, respectively.
			?>
			<script type="text/javascript">
			//<![CDATA[
				var status_cb = document.getElementById("<?php echo $prefix ?>_change_status");
				var stream_wf_cb = document.getElementById("<?php echo $prefix ?>_change_status_stream_workflow");
				var stream_byp_cb = document.getElementById("<?php echo $prefix ?>_change_status_stream_bypass");
				var editor_comment = document.getElementById("<?php echo $prefix ?>_workflow_approval_request_comment");

				if (stream_byp_cb !== null && stream_byp_cb !== undefined &&
					 stream_wf_cb !== null && stream_wf_cb !== undefined) {
					if (stream_wf_cb.options.length == 0) {
						stream_wf_cb.style.display = 'none';
					} else {
						stream_byp_cb.style.display = 'none';
					}//end if

					status_cb.onchange = function() {
						if ((this.value & <?php echo SQ_SC_STATUS_PENDING ?>) > 0) {
							stream_wf_cb.disabled = false;
							stream_wf_cb.style.display = 'inline';
							stream_byp_cb.style.display = 'none';
						} else {
							stream_wf_cb.disabled = true;
						}

						if ((this.value & <?php echo SQ_SC_STATUS_ALL_APPROVED ?>) > 0) {
							stream_byp_cb.disabled = false;
							stream_byp_cb.style.display = 'inline';
							stream_wf_cb.style.display = 'none';
						} else {
							stream_byp_cb.disabled = true;
						}
						if (typeof(editorCommenting) != "undefined") {
							editorCommenting(status_cb);
						}
						checkStatusDisplayWarning();
					}
				}//end if
			// ]]></script>
			<?php
		}
		echo '&nbsp;';
		if($is_bundle) {
			check_box($prefix.'_change_status_all_children');
		}
		else {
			check_box($prefix.'_change_status_all_children', 1, FALSE, 'checkStatusDisplayWarning()');
		}
		label(translate('Cascade Status Change'), $prefix.'_change_status_all_children');


		?>
		<script type="text/javascript">
		//<![CDATA[
			function checkStatusDisplayWarning()
			{
				var prefix = "<?php echo $prefix; ?>";
				// check if we are display warning for status change or new future status change

				var status_cb_ele = document.getElementById(prefix + '_change_status');
				var status_cascade_ele = document.getElementById(prefix + '_change_status_all_children');
				var note_text = document.getElementById('warning_message_for_live_plus_cascade');
				var current_status = <?php echo $owner->status; ?>;


				// display warning message for cascading live status
				if ((status_cb_ele.value == <?php echo SQ_STATUS_LIVE ?> || current_status == <?php echo SQ_STATUS_LIVE ?>) && status_cascade_ele.checked) {
					note_text.style.display="block";
				} else {
					note_text.style.display="none";
				}

				// display warning message for cascading with stream
				if(status_cb_ele.value & <?php echo SQ_SC_STATUS_PENDING ?> || status_cb_ele.value & <?php echo SQ_SC_STATUS_ALL_APPROVED ?> ) {
					var note_text = document.getElementById('warning_message_for_stream_cascade');
					if (status_cascade_ele.checked) {
						note_text.style.display="block";
					} else {
						note_text.style.display="none";
					}
				}

			}

			function checkStatusDisplayWarningFutureStatus()
			{
				var prefix = "<?php echo $prefix; ?>";
				// check if we are display warning for status change or new future status change
				var status_cb_ele = document.getElementById(prefix + '_new_fs_status');
				var status_cascade_ele = document.getElementById(prefix + '_new_fs_change_status_all_children');
				var note_text = document.getElementById('warning_message_for_live_plus_cascade_fs');
				var current_status = <?php echo $owner->status; ?>;

				// display warning message for cascading live status
				if ((status_cb_ele.value == <?php echo SQ_STATUS_LIVE ?> || current_status == <?php echo SQ_STATUS_LIVE ?>) && status_cascade_ele.checked) {
					note_text.style.display="block";
				} else {
					note_text.style.display="none";
				}

			}

		// ]]>
		</script>
		<?php
		if ($editor_commenting) {
			ob_start();
				echo '<div id="'.$prefix.'_workflow_approval_request_comment_div" style="display:none"><br /><br />';
				text_area($prefix.'_workflow_approval_request_comment', '', 60, 6, 0, '  id="'.$prefix.'_workflow_approval_request_comment"  placeholder="Enter comment"');
				echo '<br /><div id="note_workflow_comment">'.translate('You can enter a comment that the approvers will see when they get notified of this request.').'</div></div>';

				?>
				<script type="text/javascript">
				//<![CDATA[
					function editorCommenting(status_cbox_ele)
					{
						var editor_comment = document.getElementById("<?php echo $prefix; ?>_workflow_approval_request_comment_div");

						if ((status_cbox_ele.value & <?php echo SQ_SC_STATUS_PENDING ?>) > 0 && (editor_comment !== null && editor_comment !== undefined)) {
							editor_comment.style.display = 'inline';
						} else if (editor_comment !== null && editor_comment !== undefined) {
							editor_comment.style.display = 'none';
						}
					}
				// ]]>
				</script>
				<?php
			ob_end_flush();
		}
	}

}//end _paintStatusChange()


/**
* Processes the interface for editing the status of an asset
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param string	$prefix	prefix for form vars
*
* @return boolean
* @access public
*/
function _processStatusChange(&$owner, &$o, $prefix)
{
	if (isset($_POST[$prefix.'_change_status'])) {
		$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? FALSE : TRUE;
		if ($_POST[$prefix.'_change_status'] > 0 || !$dependants_only) {
			$new_status = 0;
			if ($_POST[$prefix.'_change_status'] == 0 && !$dependants_only) {
				$new_status = $owner->status;
			} else if ($_POST[$prefix.'_change_status'] > 0) {
				$new_status = $_POST[$prefix.'_change_status'];
			}

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $owner->id, 'new_status' => $new_status, 'dependants_only' => $dependants_only);
			if (isset($_POST[$prefix.'_change_status_stream'])) {
				if ($_POST[$prefix.'_change_status_stream'] !== 'SQ_USE_DEFAULT') {
					$vars['workflow_stream'] = $_POST[$prefix.'_change_status_stream'];
				}
			}

			if (isset($_POST[$prefix.'_workflow_approval_request_comment']) && $_POST[$prefix.'_workflow_approval_request_comment'] !== '') {
				$vars['workflow_editor_comment'] = $_POST[$prefix.'_workflow_approval_request_comment'];
			}

			$hh->queueHipo('hipo_job_edit_status', $vars);
		}
	}

}//end _processStatusChange()


/**
* Processes the interface for editing details
* returns TRUE if there is a change
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processDetails(&$owner, &$o, &$ei)
{
	// no static details screen fields relevant to an uncreated asset
	if (empty($owner->id)) return FALSE;

	$root = $ei->findScreen('static_screen_details');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();

	$is_updated = FALSE;

	foreach ($root->section as $section) {

		if (!$ei->_getAccess($owner, $section, $prefix)) {
			continue;
		}

		if (!($num_fields = count($section->children()))) {
			continue;
		}

		foreach ($section->field as $field) {

			switch (strtolower($field->attributes()->code)) {

				case 'status_change' :
					///// IMMEDIATE STATUS CHANGES ////
					$is_updated = _processDetailsStatusChange($owner, $o, $prefix) || $is_updated;
				break;

				case 'status_future_status' :
					///// FUTURE STATUS CHANGES ////
					$is_updated = _processDetailsFutureStatusCurrent($owner, $o, $prefix) || $is_updated;
					$is_updated = _processDetailsFutureStatusNew($owner, $o, $prefix) || $is_updated;
				break;

				case 'thumbnail_change' :
					///// THUMBNAIL CHANGES ////
					$is_updated = _processDetailsThumbnailChange($owner, $o, $prefix) || $is_updated;
				break;

			}//end switch on code

		}//end foreach field

	}//end foreach section

	return ($is_updated);

}//end processDetails()


/**
* Register a Simple Edit interface keyword
*
* Return TRUE if the keyword replacements list was changed
*
* @param Asset_Edit_Interface	$ei			Asset edit interface to add the keywords to
* @param array					$keywords	List of keywords
* @param string					$type		Keyword type (eg. 'fields', 'sections')
* @param string					$name		Keyword name
* @param string					$content	Content to make a keyword
*
* @return boolean
*/
function _paintDetailsRegisterKeyword(Asset_Edit_Interface $ei, $keywords, $type, $name, $content)
{
	// We are not searching for keywords at all
	if ($keywords === NULL) return FALSE;

	// We are not searching for this keyword
	if (isset($keywords[$type][$name]) === FALSE) {
		return FALSE;
	}

	$ei->_tmp['layout_keyword_replacements'][$keywords[$type][$name]] = $content;
	return TRUE;

}//end _paintDetailsRegisterKeyword()


//--        PAINTING FUNCTIONS        --//


/**
* Paint the "Current Status" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsCurrentStatus(Asset $owner, Backend_Outputter $o, $prefix)
{
	echo get_asset_status_icon($owner->status);
	echo ' <span class="sq-status-description">'.$owner->getStatusDescription().'</span>';

	return FALSE;

}//end _paintDetailsCurrentStatus()



/**
* Paint the "Change Status" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsChangeStatus(Asset $owner, Backend_Outputter $o, $prefix)
{
	// only print this field if the status can be changed
	$statii = $owner->getAvailableStatii();
	$needs_commit = FALSE;

	if (empty($statii) === FALSE) {
		// if this asset is in workflow we do not let the change the status here
		// because they need to go off to the workflow screen
		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);
		if (!empty($running_schemas)) {
			echo sprintf(translate('<i>This asset is currently in workflow. Approval or rejection of the changes must be done on the <a href="%s">asset workflow screen</a></i>.'), $owner->getBackendHref('workflow'));

		} else {
			_paintStatusChange($owner, $o, $prefix, $statii);
			$needs_commit = TRUE;
			$status = $owner->getStatus();
			$note = $status->getStatusChangeMessage();
			if ($note) {
				$o->note($note);
			}
			echo '<div id="warning_message_for_live_plus_cascade" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> Cascading Live status to child assets may result in unintended canceling of Safe Editing of those children which can result in loss of content.').'</div>';

			echo '<div id="warning_message_for_stream_cascade" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> Cascading this workflow initiation status to child assets will initiate the workflow for all children that are in a workflow ready state. The chosen workflow stream will also be cascaded if applicable.').'</div>';

		}

	}

	return $needs_commit;

}//end _paintDetailsChangeStatus()


/**
* Paint the "Current Future Statuses" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsFutureStatusCurrent(Asset $owner, Backend_Outputter $o, $prefix)
{
	// this makes assumptions about how to get the future status jobs for a particular asset, which is the domain of the cron manager.
	// The problem is that when we ask the future status job to get a list of jobs it, in turn, asks the cron manager to get a list of
	// jobs, which is turn asks the asset manager to get a list of links under the cron manager that are future status jobs.
	// In a system with a large number of future status jobs, the details screen will become very slow to load.
	$fs_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_LINK_NOTICE, 'cron_job_future_status', TRUE, 'minor');

	if (!empty($fs_links)) {
		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header"><?php echo translate('Status'); ?></th>

				<th class="sq-backend-table-header"><?php echo translate('When'); ?></th>

				<th align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Cascade?'); ?></th>

				<th align="center" class="sq-backend-table-header" style="text-align: center;"><?php echo translate('Delete?'); ?></th>

			</tr>
			<?php
			for ($k = 0; $k < count($fs_links); $k++) {
				$fs_job = $GLOBALS['SQ_SYSTEM']->am->getAsset($fs_links[$k]['majorid'], 'cron_job_future_status');
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php echo $fs_job->statusName(); ?>
					</td>
					<td class="sq-backend-table-cell">
						<?php echo $fs_job->readableWhen(); ?>
					</td>
					<td align="center" class="sq-backend-table-cell">
					<?php
						if ($fs_job->attr('dependants_only')) {
							echo translate('No');

						} else {
							echo translate('Yes');

						}
					?>
					</td>
					<td align="center" class="sq-backend-table-cell">
					<?php
						if ($fs_job->canDelete()) {
							check_box($prefix.'_remove_fs[]', $fs_job->id);
						} else {
							echo '&nbsp;';
						}
					?>
					</td>
				</tr>
				<?php
			}//end for
			?>
		</table>
		<?php

	}//end if

	return FALSE;

}//end _paintDetailsFutureStatusCurrent()


/**
* Paint the "New Future Status" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsFutureStatusNew(Asset $owner, Backend_Outputter $o, $prefix)
{
	if ($owner->writeAccess() === FALSE) return FALSE;

	$needs_commit = FALSE;

	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');

	$cron_epoc = (int)$cron_mgr->attr('epoch');
	$cron_refresh = (int)$cron_mgr->attr('refresh_time');

	if ($cron_epoc > 0 && $cron_refresh > 0) {
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');
		$fs = new Cron_Job_Future_Status();
		$edit_fns = $fs->getEditFns();

		$edit_fns->paintInLineBackend($fs, $o, $prefix.'_new_fs');

		$o->note(sprintf(translate('The Scheduled Jobs Manager runs every <b>%1$s</b>. It will next run on the <b>%2$s</b>.'), $cron_mgr->readableRefreshTime(), readable_datetime($cron_mgr->timeOfNextRun())));

		$needs_commit = TRUE;
	} else {
		echo translate('The Scheduled Jobs Manager has not been configured. No future statuses can be set for this asset.');

	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

	return $needs_commit;

}//end _paintDetailsFutureStatusNew()


/**
* Paint the "Change Thumbnail" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsThumbnailChange(Asset $owner, Backend_Outputter $o, $prefix)
{
	$edit_fns = $owner->getEditFns();
	$edit_fns->paintNoticeLinkedAsset($owner, 'thumbnail', Array('image' => 'D'));

	return TRUE;

}//end _paintDetailsThumbnailChange()


/**
* Paint the "Current Thumbnail" section of the Details screen.
*
* Return TRUE if the commit button should be activated.
*
* @param Asset				$owner	The asset being painted
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _paintDetailsThumbnailCurrent(Asset $owner, Backend_Outputter $o, $prefix)
{
	$thumbnail = NULL;
	if ($owner->useSystemVersion()) {
	    $notice_links = unserialize(file_to_string($owner->data_path.'/.sq_notice_links'));
	    foreach ($notice_links as $link) {
		 if ($link['value'] == 'thumbnail') {
		     $thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		     break;
		 }
	    }
	}
	else {
	    $link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'image', FALSE, 'thumbnail');
	    if (!empty($link)) {
		$thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
	    }
	}

	if($thumbnail) {
	    $edit_fns = $thumbnail->getEditFns();
	    $edit_fns->printExistingFileInfo($thumbnail);
	} else {
	    echo translate('No thumbnail attached');

	}

	return FALSE;

}//end _paintDetailsThumbnailCurrent()


//--        PROCESSING FUNCTIONS        --//


/**
* Process the "Status Change" section of the Details screen.
*
* In this case, the status change should only be performed on this screen if
* there is no workflow running on the asset. (Belongs on the Workflow screen
* otherwise.)
*
* Return TRUE if this was completed successfully
*
* @param Asset				$owner	The asset being processed
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _processDetailsStatusChange(Asset $owner, Backend_Outputter $o, $prefix)
{
	$is_updated = FALSE;

	$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
	$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);
	if (empty($running_schemas)) {
		_processStatusChange($owner, $o, $prefix);
		$is_updated = TRUE;
	}

	return $is_updated;

}//end _processDetailsStatusChange()


/**
* Process the "*" section of the Details screen.
*
* Return TRUE if this was completed successfully
*
* @param Asset				$owner	The asset being processed
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _processDetailsFutureStatusCurrent(Asset $owner, Backend_Outputter $o, $prefix)
{
	$is_updated = FALSE;

	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if ($cron_mgr === NULL) return FALSE;

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

	if (!empty($_POST[$prefix.'_remove_fs'])) {
		$fs_jobs = Cron_Job_Future_Status::getActiveJobs($owner);
		if (!empty($fs_jobs)) {

			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($cron_mgr->id, 'links')) {
				for ($i = 0; $i < count($fs_jobs); $i++) {
					if (in_array($fs_jobs[$i]->id, $_POST[$prefix.'_remove_fs'])) {
						if ($cron_mgr->removeJob($fs_jobs[$i])) {
							$is_updated = TRUE;
						}
					}// end if
				}// end for

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($cron_mgr->id, 'links');

			} else {
				trigger_localised_error('SYS0232', E_USER_NOTICE);

			}// end if

		}// end if
	}// end if

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

	return $is_updated;

}//end _processDetailsFutureStatusCurrent()


/**
* Process the "*" section of the Details screen.
*
* Return TRUE if this was completed successfully and there were changes
*
* @param Asset				$owner	The asset being processed
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _processDetailsFutureStatusNew(Asset $owner, Backend_Outputter $o, $prefix)
{
	$is_updated = FALSE;

	if ($owner->writeAccess('') === FALSE) return FALSE;

	$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
	if ($cron_mgr === NULL) return FALSE;

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('cron_job_future_status');

	$fs = new Cron_Job_Future_Status();
	$edit_fns = $fs->getEditFns();
	if ($fs->setAssetToUpdate($owner) && $edit_fns->processInLineBackend($fs, $o, $prefix.'_new_fs')) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($cron_mgr->addJob($fs, $GLOBALS['SQ_SYSTEM']->user)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$is_updated = TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

	return $is_updated;

}//end _processDetailsFutureStatusNew()


/**
* Process the "*" section of the Details screen.
*
* Return TRUE if this was completed successfully and there were changes
*
* @param Asset				$owner	The asset being processed
* @param Backend_Outputter	$o		Outputter class used for managing this screen
* @param string				$prefix	Prefix for form elements
*
* @return boolean
*/
function _processDetailsThumbnailChange(Asset $owner, Backend_Outputter $o, $prefix)
{
	$is_updated = FALSE;

	$edit_fns = $owner->getEditFns();
	if ($edit_fns->processNoticeLinkedAsset($owner, 'thumbnail')) {
		$is_updated = TRUE;
	}

	return $is_updated;

}//end _processDetailsThumbnailChange()


?>
