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
* $Id: trigger_action_execute_database_query.inc,v 1.8 2013/05/31 05:39:17 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/email_format/email_format.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Executes_Database_Query
*
* A trigger action runs a database query
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Trigger_Action_Execute_Database_Query extends Trigger_Action
{


	/**
	* Execute this action
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$options = Array(
					'details'	=> translate('Database Details'),

					'DSN'		=> translate('DSN'),

					'connector'	=> translate('DB Connector'),

				   );
		$method		= array_get_index($settings, 'method', '');
		$type		= array_get_index($settings, 'type', 'pgsql');
		$database	= array_get_index($settings, 'database', '');
		$host		= array_get_index($settings, 'host', '');
		$dsn		= array_get_index($settings, 'dsn', '');
		$connector	= array_get_index($settings, 'connector', '0');
		$username	= array_get_index($settings, 'username', '');
		$password	= array_get_index($settings, 'password', '');
		$sql		= array_get_index($settings, 'sql', '');
		$prefix		= array_get_index($settings, 'prefix', date('dmY'));
		if (!array_key_exists($method, $options)) {
			$method = '';
		}//end if

		// Load the current asset
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				$state['asset'] = NULL;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}//end if
		}//end if

		// Find a DSN
		$actual_dsn = '';

		// Try the DB Connector
		if ($method == 'connector' && !empty($connector)) {
			$connector = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector);
			if (!empty($connector)) {
				$actual_dsn = $connector->getDSN();
			}//end if
		}//end if

		// Try the DSN string
		if ($method == 'dsn' && !empty($dsn)) {
			$actual_dsn = $dsn;
		}//end if

		// Try database details
		if ($method == 'details' && !empty($type) && !empty($host) && !empty($database)) {
			switch ($type) {
				case 'pgsql':
					$actual_dsn = 'pgsql:';
				break;
				case 'mysql':
					$actual_dsn = 'mysql:';
				break;
			}
			$actual_dsn .= "dbname=$database;host=$host";
		}//end if

		// Nothing worked, don't execute
		if (empty($actual_dsn)) return FALSE;

		if ($method == 'connector') {
			// DSN is already properly formed
			$connect_dsn = $actual_dsn;
		} else {
			$connect_dsn = Array('DSN' => $actual_dsn, 'user' => $username, 'password' => $password, 'type' => $type);
		}//end if

		// Try to connect
		try {
			$connection = MatrixDAL::dbConnect($connect_dsn, 'trigger_action_db');
		} catch (Exception $e) {
			return FALSE;
		}

		// Try to execute the SQL
		if ($connection === FALSE) return FALSE;
		$sql = trim($sql, ';');

		// Replace keywords
		$matches = Array();
		$keyword_wrapper = '%';
		$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';
		preg_match_all ('/'.$keyword_pattern.'/', $sql, $matches, PREG_PATTERN_ORDER);

		if (!isset($matches[2])) $matches[2] = Array();

		$keywords =& $matches[2];

		$patterns = Array();
		$replacements = Array();
		$bind_vars = Array();
		foreach ($keywords as $id => $keyword) {
			$value = '';
			if (!is_null($state['asset'])) {
				$value = $state['asset']->getKeywordReplacement($keyword);
			}//end if
			if ($value != $keyword_wrapper.$keyword.$keyword_wrapper) {
				// Keyword cannot be found, don't bother doing anything else
				$value = str_replace('$', '\$', $value);
				$replacements[] = ':'.$keyword.$id;
				$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
				$bind_vars[$keyword.$id] = $value;
			}//end if
		}//end foreach

		// Treat the global keywords separately
		$includes_globals = FALSE;
		foreach(array_unique($keywords) as $keyword) {
			if (0 === strpos($keyword, 'globals_')) {
				$whole_keyword = $keyword_wrapper.$keyword.$keyword_wrapper;
				$sql = str_replace($whole_keyword, MatrixDAL::quote($whole_keyword, array_get_index($connect_dsn, 'type', NULL)), $sql);
				$includes_globals = TRUE;
			}//end if
		}//end foreach

		$sql = preg_replace($patterns, $replacements, $sql, '1');
		if($includes_globals) $sql = replace_global_keywords($sql);

		// Check for wildcards on the bind vars
		foreach ($bind_vars as $bind_var => $bind_value) {
			// Wild card at the start
			if (preg_match('/%:'.$bind_var.'/i', $sql) != FALSE) {
				$sql = str_replace('%:'.$bind_var, ':'.$bind_var, $sql);
				$bind_vars[$bind_var] = '%'.$bind_vars[$bind_var];
			}//end if

			// Wild card at the end
			if (preg_match('/'.$bind_var.'%/i', $sql) != FALSE) {
				$sql = str_replace(':'.$bind_var.'%', ':'.$bind_var, $sql);
				$bind_vars[$bind_var] = $bind_vars[$bind_var].'%';
			}//end if
		}//end foreach

		try {
			if (!isset($GLOBALS['SQ_SYSTEM']->tm) || !($GLOBALS['SQ_SYSTEM']->tm instanceof Transaction_Manager)) {
				require_once SQ_INCLUDE_PATH.'/transaction_manager.inc';
				$GLOBALS['SQ_SYSTEM']->tm = new Transaction_Manager();
			}
			MatrixDAL::changeDb('trigger_action_db');
			$GLOBALS['SQ_SYSTEM']->tm->begin('trigger_action_db');
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			$GLOBALS['SQ_SYSTEM']->tm->rollback('trigger_action_db');
			MatrixDAL::restoreDb();
			trigger_localised_error('DATA0001', sprintf(translate('SQL Execution Error:<br/>Error: %1$s<br/>SQL: %2$s'), $e->getMessage(), $sql), E_USER_WARNING);
			return FALSE;
		}

		// commit the transaction
		$GLOBALS['SQ_SYSTEM']->tm->commit('trigger_action_db');
		MatrixDAL::restoreDb();

		// Return a response
		return TRUE;

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
		$method		= array_get_index($settings, 'method', 'details');
		$type		= array_get_index($settings, 'type', 'pgsql');
		$database	= array_get_index($settings, 'database', '');
		$host		= array_get_index($settings, 'host', '');
		$dsn		= array_get_index($settings, 'dsn', '');
		$connector	= array_get_index($settings, 'connector', '0');
		$username	= array_get_index($settings, 'username', '');
		$password	= array_get_index($settings, 'password', '');
		$sql		= array_get_index($settings, 'sql', '');
		$options = Array(
					'details'	=> translate('Database Details'),

					'DSN'		=> translate('DSN'),

					'connector'	=> translate('DB Connector'),

				   );
		if (!array_key_exists($method, $options) || empty($method)) {
			$method = 'details';
		}//end if
		$enable  = ' style="display:table-row;visibility:visible;"';
		$disable = ' style="display:none;visibility:hidden;"';
		ob_start();
			?>
			<script type="text/javascript">
				function toggleDatabaseSetup(connectionType)
				{
					// Disbale all by default
					var username = document.getElementById('trigger_action_user_username');
					var password = document.getElementById('trigger_action_user_password');
					var dbdetail = document.getElementById('trigger_action_database_details');
					var dbcomdsn = document.getElementById('trigger_action_dsn');
					var dbconnec = document.getElementById('trigger_action_connector');
					dbdetail.style.display = "none";
					dbdetail.style.visibility = "hidden";
					dbcomdsn.style.display = "none";
					dbcomdsn.style.visibility = "hidden";
					dbconnec.style.display = "none";
					dbconnec.style.visibility = "hidden";
					username.style.display = "table-row";
					username.style.visibility = "visible";
					password.style.display = "table-row";
					password.style.visibility = "visible";

					if (connectionType == "dsn") {
						dbcomdsn.style.display = "table-row";
						dbcomdsn.style.visibility = "visible";
					} else if (connectionType == "connector") {
						dbconnec.style.display = "table-row";
						dbconnec.style.visibility = "visible";
						username.style.display = "none";
						username.style.visibility = "hidden";
						password.style.display = "none";
						password.style.visibility = "hidden";
					} else {
						dbdetail.style.display = "table-row";
						dbdetail.style.visibility = "visible";
					}
				}
			</script>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Connection Type'); ?></th>

					<td><?php
							if ($write_access) {
								combo_box($prefix.'[method]', $options, FALSE, $method, '1', 'onchange="toggleDatabaseSetup(document.getElementById(\''.$prefix.'[method]\').value)"');
							} else {
								echo $options[$method];
							}//end if
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Connection Setup'); ?></th>

					<td>
						<table class="sq-backend-table">
							<?php $style = ($method != 'connector') ? $enable : $disable; ?>
							<tr id="trigger_action_user_username" <?php echo $style; ?>>
								<th style="width:100px;"><?php echo translate('User Name');?></th>

								<td><?php
										if ($write_access) {
											text_box($prefix.'[username]', $username, 35);
										} else {
											echo $username;
										}//end if
									?>
								</td>
							</tr>
							<tr id="trigger_action_user_password" <?php echo $style; ?>>
								<th style="width:100px;"><?php echo translate('Password');?></th>

								<td><?php
										if ($write_access) {
											password_box($prefix.'[password]', $password, 35);
										} else {
											echo translate('(Password not displayed)');

										}//end if
									?>
								</td>
							</tr>
							<?php $style = ($method == 'details') ? $enable : $disable; ?>
							<tr id="trigger_action_database_details" <?php echo $style; ?>>
								<th style="width:100px;"><?php echo translate('Database Details'); ?></th>

								<td>
									<table class="sq-backend-table">
										<tr>
											<th><?php echo translate('Database Type');?></th>

											<td>
												<?php
													$db_options = Array('pgsql' => 'PostgreSQL', 'oci' => 'Oracle', 'mysql' => 'MySQL');
													if (!array_key_exists($type, $db_options) || empty($type)) {
														$type = 'pgsql';
													}//end if
													if ($write_access) {
														combo_box($prefix.'[type]', $db_options, FALSE, $type);
													} else {
														echo $db_options[$type];
													}//end if
												?>
											</td>
										</tr>
										<tr>
											<th><?php echo translate('Database Name');?></th>

											<td><?php
													if ($write_access) {
														text_box($prefix.'[database]', $database, 35);
													} else {
														echo $database;
													}//end if
												?>
											</td>
										</tr>
										<tr>
											<th><?php echo translate('Host Name'); ?></th>

											<td><?php
													if ($write_access) {
														text_box($prefix.'[host]', $host, 35);
													} else {
														echo $host;
													}//end if
												?>
											</td>
										</tr>
									</table>
								</td>
							</tr>
							<?php $style = ($method == 'dsn') ? $enable : $disable; ?>
							<tr id="trigger_action_dsn" <?php echo $style; ?>>
								<th style="width:100px;"><?php echo translate('DSN'); ?></th>

								<td><?php
										if ($write_access) {
											text_box($prefix.'[dsn]', $dsn, 70);
										} else {
											echo $dsn;
										}//end if
									?>
								</td>
							</tr>
							<?php $style = ($method == 'connector') ? $enable : $disable; ?>
							<tr id="trigger_action_connector" <?php echo $style; ?>>
								<th style="width:100px;"><?php echo translate('DB Connector'); ?></th>

								<td><?php
										if ($write_access) {
											asset_finder($prefix.'[connector]', $connector, Array('db_connector'=>'D'));
										} else {
											if (!empty($connector)) {
												echo get_asset_tag_line($connector);
											} else {
												echo translate('No asset selected');

											}//end if
										}//end if
									?>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('SQL Query'); ?></th>

					<td><?php
							if ($write_access) {
								text_area($prefix.'[sql]', $sql, 100, 40, 0, 'style="width: 80%; height: 30em; font-family: \'Courier New\', Courier, monospace; display: block"');
							} else {
								echo $sql;
								echo "<br />\n";
							}//end if
							echo translate('<strong>Note:</strong> You can use keywords of the current asset (eg. %%asset_assetid%%) and global keywords (eg. %%globals_user_assetid%%) in the SQL Query.');

						?>
					</td>
				</tr>
			</table>

			<?php
			$interface = ob_get_contents();
		ob_end_clean();

		return $interface;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{

		// Save the request data to the settings array
		$settings['method']		= array_get_index($request_data, 'method', 'details');
		$settings['type']		= array_get_index($request_data, 'type', 'pgsql');
		$settings['database']	= array_get_index($request_data, 'database', '');
		$settings['host']		= array_get_index($request_data, 'host', '');
		$settings['dsn']		= array_get_index($request_data, 'dsn', '');
		$settings['connector']	= array_get_index((array_get_index($request_data, 'connector', Array())), 'assetid', '0');
		$settings['username']	= array_get_index($request_data, 'username', '');
		$settings['password']	= array_get_index($request_data, 'password', '');
		$settings['sql']		= array_get_index($request_data, 'sql', '');

		return FALSE;

	}//end processInterface()


}//end class

?>
