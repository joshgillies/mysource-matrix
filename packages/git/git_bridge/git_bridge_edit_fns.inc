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

require_once SQ_PACKAGES_PATH.'/filesystem/file_bridge/file_bridge_edit_fns.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Git_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.net>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage git
*/
class Git_Bridge_Edit_Fns extends File_Bridge_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Git_Bridge_Edit_Fns()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the current repo name
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRepoName($asset, &$o, $prefix)
	{
		echo $asset->getRepoUrl(FALSE);

		return TRUE;

	}//end paintUpdateRepo()


	/**
	* Prints the clone URL input
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintGitUrl($asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$value = $asset->attr('clone_url');
		if ($wa) {
			text_box($prefix.'_git_url', $value, 50);

			$git_exists = is_executable(SQ_TOOL_GIT_PATH);
			// Show clone repo button only if current repo is different from the one specified
			$clone_url = $asset->fullCloneUrl();
			if (!empty($clone_url) && $git_exists && !$asset->repoLocked() && $clone_url != $asset->getRepoUrl()) {
				?>
				<style type="text/css">
					#<?php echo $prefix; ?>_clone_updating_status {
						background: url(<?php echo sq_web_path('lib').'/web' ?>/images/icons/asset_map/spinner.gif) no-repeat center -100px;
						display: inline-block;
						min-width: 20px;
					}
					#<?php echo $prefix; ?>_clone_updating_status.loading {
						background-position: center left;
					}
				</style>

				<script>

					function _<?php echo $prefix; ?>_clone_repo()
					{
						document.getElementById("<?php echo $prefix; ?>_clone_updating_status").innerHTML = '&nbsp;';
						document.getElementById("<?php echo $prefix; ?>_clone_updating_status").className = 'loading';
						var button =  document.getElementById("<?php echo $prefix; ?>_clone_repo_btn");
						button.disabled = true;

						var checkStatus = null;

						// Make an ajax request to clone the repo
						// If the request timeouts, initiate the polling request the check the lock status
						ajax_request(
								'../?a=<?php echo $asset->id; ?>&SQ_GIT_BRIDGE_ACTION=clone',
								printResponseStatus,
								'GET', null, true, 30000, // Timeout after 30 seconds and execute the following callback
								function () {
									checkStatus = setInterval(function() {
										ajax_request(
											'../?a=<?php echo $asset->id; ?>&SQ_GIT_BRIDGE_ACTION=lock_status',
											printResponseStatus
										)}, 2000 // Check repo lock status every 2 secs
									);
								}
						);

						// Print the ajax request response
						function printResponseStatus(response) 
						{
							if (response) {
								response = JSON.parse(response);
								if (response.action_type == 'clone' || response.locked === false) {
									if (checkStatus) {
										window.clearInterval(checkStatus);
									}

									var error_occured = response.errors && response.errors.length;
									document.getElementById("<?php echo $prefix; ?>_clone_response").innerHTML = error_occured ? '<pre>'+response.errors.join('<br/>')+'</pre>' : '';
									document.getElementById("<?php echo $prefix; ?>_clone_updating_status").innerHTML = error_occured ? 'Failed.' : 'Done.';
									document.getElementById("<?php echo $prefix; ?>_clone_updating_status").className = '';
									button.disabled = false;

									if (!error_occured) {
										// Refresh the frame and the git bridge asset in the asset map
										if (parent.frames["sq_sidenav"] && parent.frames["sq_sidenav"].reload_assets) {
											parent.frames["sq_sidenav"].reload_assets("<?php echo $asset->id; ?>");
										}
										window.location = window.location.href;
									}
								}
							}//end if response

						}// end printResponseStatus()

					}//end function

				</script>
				<?php

				normal_button($prefix.'_clone_repo_btn', "Clone Repo", '_'.$prefix.'_clone_repo();');
				echo '<span id="'.$prefix.'_clone_updating_status">&nbsp;</span>';
				echo '<span id="'.$prefix.'_clone_response"></span>';
			}
			if (!$git_exists) {
				echo '<div style="display: block;" class="sq-warning-message top-margin"><strong>Warning:</strong>This asset requires Git tool to clone this remote Git URL to the filesystem. Path to Git binary defined in the External Tool configuration is not valid.</div>';
			}
		} else {
			echo $value;
		}

		return $wa;

	}//end paintGitUrl()


	/**
	* Process the interface of sub directories selection
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processGitUrl(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$success = TRUE;
		if ($wa && isset($_POST[$prefix.'_git_url'])) {
			$success = $asset->setAttrValue('clone_url', trim($_POST[$prefix.'_git_url']));
		}

		return $success;

	}//end processGitUrl()


	/**
	* Prints the interface for Update Repo checkbox
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUpdateRepo($asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			?>
			<style type="text/css">
				#<?php echo $prefix; ?>_updating_status {
					background: url(<?php echo sq_web_path('lib').'/web' ?>/images/icons/asset_map/spinner.gif) no-repeat center -100px;
					display: inline-block;
					min-width: 20px;
				}
				#<?php echo $prefix; ?>_updating_status.loading {
					background-position: center left;
				}
			</style>

			</style>
			<script>
				function _<?php echo $prefix; ?>_update_repo()
				{
					var button =  document.getElementById("<?php echo $prefix; ?>_update_repo_btn");
					button.disabled = true;
					document.getElementById("<?php echo $prefix; ?>_updating_status").className = 'loading';
					document.getElementById("<?php echo $prefix; ?>_updating_status").innerHTML = '&nbsp;';
					ajax_request(
						'../?a=<?php echo $asset->id; ?>&SQ_GIT_BRIDGE_ACTION=pull',
						function (response) {
							response = JSON.parse(response);
							if (response.update_date) {
								document.getElementById("<?php echo $prefix; ?>_update_msg").firstElementChild.innerHTML = response.update_date;
							}
							var error_occured = response.errors && response.errors.length;
							document.getElementById("<?php echo $prefix; ?>_updating_response").innerHTML = error_occured ? '<pre>'+response.errors.join('<br/>')+'</pre>' : '';
							document.getElementById("<?php echo $prefix; ?>_updating_status").innerHTML = error_occured ? 'Failed.' : 'Done.';
							document.getElementById("<?php echo $prefix; ?>_updating_status").className = '';
							button.disabled = false;
						}
					);

				}//end function
			</script>
			<?php
			normal_button($prefix.'_update_repo_btn', "Update Now", '_'.$prefix.'_update_repo();');
			echo '<span id="'.$prefix.'_updating_status">&nbsp;</span>';
			echo '<span id="'.$prefix.'_updating_response"></span>';
		}
		echo '<div id="'.$prefix.'_update_msg" class="sq-backend-smallprint">Last updated on <strong>'.$asset->latestRepoUpdateDate().'</strong></div>';

		return $wa;

	}//end paintUpdateRepo()


	/**
	* Paint the interface for sub directories selection
	*
	* @param object	$asset	the file asset
	* @param object	$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubDirectories($asset, $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$value = $asset->attr('sub_dirs');

		if ($wa) {
			text_box($prefix.'_sub_dirs', $value, 70);
		} else {
			echo $value;
		}

		return $wa;

	}//end paintSubDirectories()


	/**
	* Process the interface of sub directories selection
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubDirectories(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$success = TRUE;
		if ($wa && isset($_POST[$prefix.'_sub_dirs'])) {
			$value = $_POST[$prefix.'_sub_dirs'];
			if ($value != $asset->attr('sub_dirs')) {
				$success = $asset->setAttrValue('sub_dirs', $value);
			}
		}

		return $success;

	}//end processSubDirectories()


	/**
	* paint the interface for Select Branch
	*
	* @param object	$asset	the file asset
	* @param object	$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSelectBranch($asset, $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$branches = $asset->getRepoBranchNames();
		if (!empty($branches)) {
			$options = Array();
			$selected = FALSE;
			foreach($branches as $branch => $is_selected) {
				$options[$branch] = $branch;
				if (!$selected && $is_selected) {
					$selected = $branch;
				}
			}
			if ($wa) {
				combo_box($prefix.'_branch', $options, FALSE, $selected);
			} else {
				echo $selected;
			}
		} else {
			echo translate('Repository is empty.');
		}

	}//end paintSelectBranch()

	/**
	* Processes the interface for Select Branch
	*
	* @param object	$asset	the file asset
	* @param object	$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSelectBranch($asset, $o, $prefix)
	{
		$success = FALSE;
		if (!empty($_POST[$prefix.'_branch']) && $asset->writeAccess('attributes')) {
			$asset->switchBranch($_POST[$prefix.'_branch']);
		}

		//$asset->updateRepo();

		return $success;

	}//end processSelectBranch()


	/**
	* Paint the usage of this Git File Bridge
	*
	* @param Design				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintUsage(Git_Bridge $asset, Backend_Outputter $o, $prefix)
	{
		$page_size = 20;

		$sql = 'SELECT majorid, minorid FROM '.SQ_TABLE_RUNNING_PREFIX.'shdw_ast_lnk WHERE minorid LIKE '.MatrixDAL::quote($asset->id . ':%');
		$links_result = MatrixDAL::executeSqlAssoc($sql);

		$o->openRaw();

		if (count($links_result) > 0) {

			$asset_links = Array();
			foreach($links_result as $link) {
				if (!isset($asset_links[$link['majorid']])) {
					$asset_links[$link['majorid']] = Array();
				}
				// Shadow bit of minor assetid contains file path
				$id_parts = explode(':', $link['minorid']);
				$asset_links[$link['majorid']][] = $id_parts[1];
			}

			$num_pages = ceil(count($asset_links) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total assets'), ($page_num - 1) * $page_size + 1, min($num_pages, $page_num * $page_size), count($asset_links));

			$asset_links = array_slice($asset_links, ($page_num - 1) * $page_size, $page_size, TRUE);

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('Go to the first page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', 1)">'.$links['first'].'</a>';

				$links['previous'] = '<a title="'.translate('Go to the previous page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';

			} else {
				$links['first'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['first'].'</span>';

				$links['previous'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['previous'].'</span>';

			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('Go to the last page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.$num_pages.')">'.$links['last'].'</a>';

				$links['next'] = '<a title="'.translate('Go to the next page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num + 1).')">'.$links['next'].'</a>';

			} else {
				$links['last'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['last'].'</span>';

				$links['next'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['next'].'</span>';

			}

			hidden_field($prefix.'_usage_page', $page_num); ?>
			<table class="sq-backend-table">
			<thead>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-header-header sq-pagination-links sq-no-side-border" style="text-align: left; width:40%"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></th>
				<th class="sq-backend-table-header-header sq-no-side-border" style="text-align: right;"><?php echo $asset_count_tag; ?></span></th>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell"><?php echo translate('Asset'); ?></th>
				<th class="sq-backend-table-cell"></th>
			</tr>
			</thead>
			<tbody>
			<?php

			$data_dir = $asset->data_path_public;
			foreach ($asset_links as $assetid => $file_paths) {
				?><tr class="sq-backend-table-row">
					<td class="sq-backend-table-cell">
					<?php
						$tag_line = get_asset_tag_line($assetid, 'details');
						echo $tag_line;

						foreach($file_paths as &$file_path) {
							if (!is_file($data_dir.'/'.$file_path)) {
								$file_path = '<em>(invalid)</em>&nbsp;' .$file_path;
							}
						}
						rsort($file_paths);
					?>
					</td>
					<td>
						<ul>
						<?php
							echo '<li>'.implode('</li><li>', $file_paths).'</li>';
						?>
						</ul>
					</td>
				</tr>
				<?php
			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

			$o->note(translate('This screen shows the assets in which the files in this Git Bridge are referenced.'));


		} else {
			$o->note(translate('<p>The files in this Git Bridge are not referenced in any assets.</p>'));

		}

		$o->closeRaw();

	}//end paintUsage()


	/**
	* Paint the interface for printing the lock status
	*
	* @param object	$asset	the file asset
	* @param object	$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRepoLocked($asset, $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$locked_time = $asset->repoLocked();
		if ($wa) {
			check_box($prefix.'_unlock_repo', 1, FALSE, 'this.nextSibling.style.display = "block";');
			echo '<div style="display: none;" class="sq-warning-message top-margin"><strong>Warning:</strong> Locked repository means the current repository is being updated. Use this option to remove the lock only if you are sure that there is no other process currently updating the repository.</div>';
		}
		echo '<div class="sq-backend-smallprint">Locked since <strong>'.$locked_time.'</strong></div>';

		return $wa;

	}//end paintRepoLocked()


	/**
	* Process the interface for process the lock status
	*
	* @param object	$asset	the file asset
	* @param object	$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processRepoLocked($asset, $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$success = TRUE;
		if ($wa && !empty($_POST[$prefix.'_unlock_repo'])) {
			$success = $asset->removeRepoLock();
		}

		return $success;

	}//end processLocked()


	/**
	* Show-if conditional: show if the Git URL is HTTPS or empty
	*
	* @param object	$asset
	*
	* @return boolean
	* @access public
	*/
	function usingHttpsUrl($asset)
	{
		$git_url = $asset->attr('clone_url');

		return empty($git_url) || strpos($git_url, 'https:') === 0;

	}//end usingHttpsUrl()


	/**
	* Show-if conditional: show if we have a repo avilable
	*
	* @param object	$asset
	*
	* @return boolean
	* @access public
	*/
	function repoAvailable($asset)
	{
		return is_executable(SQ_TOOL_GIT_PATH) && $asset->repoAvailable() && !$asset->repoLocked();

	}//end repoAvailable()


	/**
	* Show-if conditional: show if we have a repo avilable
	*
	* @param object	$asset
	*
	* @return boolean
	* @access public
	*/
	function repoLocked($asset)
	{
		return $asset->repoLocked();

	}//end repoLocked()


}//end class

?>
