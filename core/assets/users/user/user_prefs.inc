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
* $Id: user_prefs.inc,v 1.8 2006/12/05 06:04:33 bcaldwell Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';

/**
* User_Prefs
*
* Purpose
*
*    Looks after the session expiry prefences for user groups
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/
class User_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_USER_SESSION_PREFS'		=> Array(
														'name'			=> 'Session Expiry Rules',
														'description'	=> 'This preference allows you to change the session settings for this user group',
														'default'		=> Array('persist' => '0', 'timeout' => '0', 'max_length' => '0'),
														'protected'		=> FALSE,
													   ),
						'SQ_USER_ASSET_MAP_LINEAGE'	=> Array(
														'name'			=> 'Asset Map Root Asset',
														'description'	=> 'This preference allows you to change the root node that is displated in the Asset Map',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
						'SQ_USER_SAFE_TYPE3_TRASH'	=> Array(
														'name'			=> 'Safe Trash',
														'description'	=> 'This preference allows you to prohibit the deletion of an asset until all notice links are removed',
														'default'		=> FALSE,
														'protected'		=> FALSE,
													   ),
					 );


	/**
	* Constructor
	*
	*/
	function User_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit session preferences
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

		if (is_null($pref) || $pref == 'SQ_USER_SESSION_PREFS') {
			if (is_null($pref)) {
				$o->openField(translate('user_prefs_session_settings_header'));
			}
			$session_settings = $this->pref_vars['SQ_USER_SESSION_PREFS']['default'];
			?>
			<style type="text/css">
				ul#user-prefs {
					list-style-type: none;
					padding-left: 8px;
					margin: 2px 0px;
				}
				ul#user-prefs li {
					white-space: nowrap;
					margin-left: 8px;
					height: 22px;
				}
				div.sq-backend-table-cell {
					width: auto;
					margin: 3px;
				}
			</style>
			<div class="sq-backend-table sq-backend-table-cell">
				<?php echo translate('end_user_session'); ?>...
				<ul id="user-prefs">
					<li>
						<?php
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS][persist]', '0', ($session_settings['persist'] == 0), '', 'id="prefs_SQ_USER_SESSION_PREFS_persist"');
							label(translate('session_persist'), 'prefs_SQ_USER_SESSION_PREFS_persist');
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(empty($session_settings['persist']) ? 'tick' : 'cross').'.gif" />';
							echo translate('session_persist');
						}
						?>
					</li>
					<li id="timeout">
						<?php
						$dur =& new Asset_Attribute_Duration(0, $session_settings['timeout']);
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS][timeout_enabled]', '1', ($session_settings['timeout'] != 0), 'toggleInputs(this.parentNode)', 'id="prefs_SQ_USER_SESSION_PREFS_timeout_enabled"');
							label(translate('session_timeout'), 'prefs_SQ_USER_SESSION_PREFS_timeout_enabled');
							echo ' ';
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(!empty($session_settings['timeout']) ? 'tick' : 'cross').'.gif" />';
							echo translate('session_timeout');
						}
						$dur->paint('prefs_dur_timeout_', !$have_lock);
						?>
					</li>
					<li id="max_length">
						<?php
						$dur =& new Asset_Attribute_Duration(0, $session_settings['max_length']);
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS][max_length_enabled]', '1', ($session_settings['max_length'] != 0), 'toggleInputs(this.parentNode)', 'id="prefs_SQ_USER_SESSION_PREFS_max_length_enabled"');
							label(translate('session_max_length'), 'prefs_SQ_USER_SESSION_PREFS_max_length_enabled');
							echo ' ';
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(!empty($session_settings['max_length']) ? 'tick' : 'cross').'.gif" />';
							echo translate('session_max_length');
						}
						$dur->paint('prefs_dur_max_length_', !$have_lock); ?>
					</li>
				</ul>
				<?php echo '*'.translate('session_timeout_explanation'); ?>
			</div>
			<?php
			if ($have_lock && $is_admin) {
				?>
				<script type="text/javascript">
					function toggleInputs(elt) {
						var inputs = elt.getElementsByTagName('INPUT');
						for (var i=0; i < inputs.length; i++) {
							if (inputs[i].type.toLowerCase() == 'text') {
								inputs[i].disabled = !inputs[i].disabled;
							}
						}
					}
					<?php
					// disable fields initially if appropriate
					foreach (Array('timeout', 'max_length') as $field) {
						if (empty($session_settings[$field])) {
							?>
							toggleInputs(document.getElementById('<?php echo $field; ?>'));
							<?php
						}
					}
					?>
				</script>
				<?php
			}
			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_LINEAGE') {
			if (is_null($pref)) {
				$o->openField(translate('user_prefs_asset_map_root_asset_header'));
			}
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php

			if ($have_lock && $is_admin) {

				if (isset($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
					$asset_map_settings = $this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'];
				} else {
					$asset_map_settings = '';
				}
				asset_finder('prefs[SQ_USER_ASSET_MAP_LINEAGE]', $asset_map_settings);
			} else {
				if (!empty($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
					echo get_asset_tag_line($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default']);
				} else {
					echo translate('root_folder');
				}
			}
			?>
			<br /><br />
			<?php
			echo translate('user_prefs_asset_map_root_asset_explanation');
			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_SAFE_TYPE3_TRASH') {
			if (is_null($pref)) {
				$o->openField('Safe Trash');
			}
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php
				$safe_trash = $this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'];
				if ($have_lock && $is_admin) {
					check_box('prefs[SQ_USER_SAFE_TYPE3_TRASH]', 1, ($safe_trash), '', 'id="prefs_SQ_USER_SAFE_TYPE3_TRASH"');
					label('Check this box if you want to prohibit the deletion of an asset until all notice links are removed', 'prefs_SQ_USER_SAFE_TYPE3_TRASH');
				} else {
					echo '<img src="'.sq_web_path('lib').'/web/images/'.($safe_trash ? 'tick' : 'cross').'.gif" />';
					echo('Check this box if you want the system to prohibit the deletion of an asset until all notice links are removed');
				}
			?>
			<br />
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if



		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit session preferences
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
		if (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
			return FALSE;
		}
		$new_prefs = Array('persist' => 0, 'timeout' => 0, 'max_length' => 0);
		if (is_null($pref) || $pref == 'SQ_USER_SESSION_PREFS') {
			$new_prefs['persist'] = !isset($_POST['prefs']['SQ_USER_SESSION_PREFS']['persist']);
			if (isset($_POST['prefs']['SQ_USER_SESSION_PREFS']['timeout_enabled'])) {
				$timeout_dur =& new Asset_Attribute_Duration();
				$timeout_dur->process('prefs_dur_timeout_');
				$new_prefs['timeout'] = $timeout_dur->value;
			}
			if (isset($_POST['prefs']['SQ_USER_SESSION_PREFS']['max_length_enabled'])) {
				$max_length_dur =& new Asset_Attribute_Duration();
				$max_length_dur->process('prefs_dur_max_length_');
				$new_prefs['max_length'] = $max_length_dur->value;
			}
			$this->pref_vars['SQ_USER_SESSION_PREFS']['default'] = $new_prefs;
		}

		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_LINEAGE') {
			// update lineage pref
			if (isset($_POST['prefs']['SQ_USER_ASSET_MAP_LINEAGE'])) {
				$this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'] = $_POST['prefs']['SQ_USER_ASSET_MAP_LINEAGE']['assetid'];
			}
		}

		$gc_min = max($new_prefs['timeout'], $new_prefs['max_length']);
		if ($gc_min > SQ_CONF_SESSION_GC_MAXLIFETIME) {
			// update the main conf
			include_once(SQ_INCLUDE_PATH.'/system_config.inc');
			$system_conf =& new System_Config();
			if (!$system_conf->canAcquireLock()) {
				trigger_localised_error('CORE0233', E_USER_NOTICE);
				return FALSE;
			}

			$system_conf->acquireLock();
			$system_conf->save(Array('SQ_CONF_SESSION_GC_MAXLIFETIME' => $gc_min + 86400), FALSE, FALSE);
			$system_conf->releaseLock();
		}

		if (is_null($pref) || $pref == 'SQ_USER_SAFE_TYPE3_TRASH') {
			// update lineage pref
			if (isset($_POST['prefs']['SQ_USER_SAFE_TYPE3_TRASH'])) {
				$this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'] = $_POST['prefs']['SQ_USER_SAFE_TYPE3_TRASH'];
			} else {
				$this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'] = FALSE;
			}
		}



		return TRUE;

	}//end processBackend()


	/**
	* Merges two group-defined preference arrays for a content type
	*
	* Note that this function is only called if there is a conflict between two groups' preferences.
	* If there is a conflict between a group's prefs and the global prefs, the group automatically wins and
	* this function is not called.
	* Array returned is of the format (prefs_var => prefs_details)
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		$res = Array();
		if (isset($prefs1['SQ_USER_SESSION_PREFS']['default']) && isset($prefs2['SQ_USER_SESSION_PREFS']['default'])) {
			// Follow the princple that the most-open option (the one with the check disabled,
			// or with the longest timeout) wins.
			$session_settings_1 = $prefs1['SQ_USER_SESSION_PREFS']['default'];
			if (is_array($prefs2['SQ_USER_SESSION_PREFS']) && isset($prefs2['SQ_USER_SESSION_PREFS']['default'])) {
				$session_settings_2 = $prefs2['SQ_USER_SESSION_PREFS']['default'];
			} else {
				$session_settings_2 = $prefs2['SQ_USER_SESSION_PREFS'];
			}
			$merged['persist'] = max($session_settings_1['persist'], $session_settings_2['persist']);
			foreach (Array('timeout', 'max_length') as $pref_comp) {
				if (0 == min($session_settings_1[$pref_comp], $session_settings_2[$pref_comp])) {
					$merged[$pref_comp] = 0;
				} else {
					$merged[$pref_comp] = max($session_settings_1[$pref_comp], $session_settings_2[$pref_comp]);
				}
			}

			$res['SQ_USER_SESSION_PREFS']['default'] = $merged;
		} else if (isset($prefs1['SQ_USER_ASSET_MAP_LINEAGE']['default']) && isset($prefs2['SQ_USER_ASSET_MAP_LINEAGE'])) {

			// asset map settings merge get the common
			$asset_map_settings1 = $prefs1['SQ_USER_ASSET_MAP_LINEAGE']['default'];
			if (is_array($prefs2['SQ_USER_ASSET_MAP_LINEAGE']) && isset($prefs2['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
				$asset_map_settings2 = $prefs2['SQ_USER_ASSET_MAP_LINEAGE']['default'];
			} else {
				$asset_map_settings2 = $prefs2['SQ_USER_ASSET_MAP_LINEAGE'];
			}

			if (empty($asset_map_settings1) && !empty($asset_map_settings2)) {
				$asset_map_settings1 = $asset_map_settings2;
				$asset_map_settings2 = '';
			}

			$res['SQ_USER_ASSET_MAP_LINEAGE']['default'] = $asset_map_settings1;

			if (!empty($asset_map_settings2)) {
				$res['SQ_USER_ASSET_MAP_LINEAGE']['default'] .= ','.$asset_map_settings2;
			}

		}

		if (isset($prefs1['SQ_USER_SAFE_TYPE3_TRASH']['default']) && isset($prefs2['SQ_USER_SAFE_TYPE3_TRASH'])) {

			// for safe trash option, if user belongs to multiple user groupS, option Yes > No
			// note that param 2 is value only
			$safe_trash1 = $prefs1['SQ_USER_SAFE_TYPE3_TRASH']['default'];
			$safe_trash2 = $prefs2['SQ_USER_SAFE_TYPE3_TRASH']['default'];
			$res['SQ_USER_SAFE_TYPE3_TRASH']['default'] = ($safe_trash1 || $safe_trash2) ? TRUE : FALSE;

		}

		return $res;

	}//end mergePrefs()


}//end class

?>
