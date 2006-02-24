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
* $Id: data_source_db_edit_fns.inc,v 1.12.2.1 2006/02/24 01:06:03 arailean Exp $
*
*/
require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';
/**
* Data_Source_DB_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_DB_Edit_Fns extends Data_Source_Edit_Fns
{
	var $dsn = null;
	var $result = null;
	var $fields = Array();
	var $selected_fields = Array();
	var $group_by = Array();
	var $conn = null;


	/**
	* Constructor
	*
	*/
	function Data_Source_DB_Edit_Fns()
	{
		$this->Data_Source_Edit_Fns();

	}//end constructor


//--        DSN        --//


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintDSN(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$database = $asset->attr('database');
		$host = $asset->attr('host');
		$user = $asset->attr('user');
		$password = $asset->attr('password');
		?>
		<table class="sq-backend-table">
			<tr>
				<td><?php echo translate('database_name');?></td>
				<td>
					<?php
					if ($wa) {
						text_box('dsn[database]', $database, 35);
					} else {
						echo $database;
					}
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('user_name');?></td>
				<td>
					<?php
					if ($wa) {
						text_box('dsn[user]', $user, 35);
					} else {
						echo $user;
					}
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('host_name');?></td>
				<td>
					<?php
					if ($wa) {
						text_box('dsn[host]', $host, 35);
					} else {
						echo $host;
					}
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('password');?></td>
				<td>
					<?php
					if ($wa) {
						password_box('dsn[password]', $password, 35);
					} else {
						echo 'Password Cannot Be Displayed';
					}
					?>
				</td>
			</tr>
		</table>
		<?php
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintDSN()


	/**
	* Processes the interface for entering DSN
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDSN(&$asset, &$o, $prefix)
	{
		$db_type = $asset->attr('db_type');
		$dsn = $asset->attr('dsn');
		$new_dsn_string = null;
		$new_dsn = $_POST['dsn'];
		if (!empty($_POST['dsn']['dsn'])) {
			$new_dsn_string = $_POST['dsn']['dsn'];
			$new_dsn['user'] = null;
			$new_dsn['password'] = null;
			$new_dsn['host'] = null;
			$new_dsn['database'] = null;
		} else {
			switch ($db_type) {
				case 'postgres':
					$new_dsn_string = "pgsql://$new_dsn[user]";
				break;
				case 'mysql':
					$new_dsn_string = "mysql://$new_dsn[user]";
				break;
			}
			!empty($new_dsn['password'])?$new_dsn_string .= ":$new_dsn[password]":null;
			$new_dsn_string .= "@$new_dsn[host]/$new_dsn[database]";
		}
		if (strcasecmp($dsn, $new_dsn_string) != 0) {
			foreach ($new_dsn as $key => $value) {
				$asset->setAttrValue($key, $value);
			}
			$asset->setAttrValue('dsn', $new_dsn_string);
			$res = $this->_resetAttributes($asset, 'reset');
			return true;
		}
		return false;

	}//end processDSN()


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintDSNAlone(&$asset, &$o, $prefix)
	{
		$dsn = $asset->attr('dsn');
		$wa = $asset->writeAccess('attributes');
		$user = $asset->attr('user');
		if ($wa) {
			text_area('dsn[dsn]', empty($user)? $dsn: '', 70, 5);
			return true;
		} else {
			if (!empty($dsn)) echo $dsn;
			return false;
		}

	}//end paintDSNAlone()


	/**
	* Processes the interface for entering DSN
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDSNAlone(&$asset, &$o, $prefix)
	{
		return false;

	}//end processDSNAlone()


//--        TABLES        --//


	/**
	* Paints the interface for selecting the tables
	*
	* @param object	&$asset	the Data_Source_DB asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTables(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$tables = $this->_getAllTables($asset);
		$mode = $asset->attr('mode');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$chosen_tables = Array();
		if (isset($_POST['tables'])) {
			$chosen_tables = array_values($_POST['tables']);
		} else {
			$chosen_tables = $asset->attr('tables');
			$chosen_tables = explode(', ', $chosen_tables);
		}
		if ($wa) {
			if (!empty($tables)) {
				combo_box('tables', $tables, true, !empty($chosen_tables) ? $chosen_tables : '', 6);
			} else {
				echo translate('no_tables_found');
			}
		} else {
			echo implode(', ', $chosen_tables);
		}
		echo ('&nbsp;');
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintTables()


	/**
	* Processes the interface for selecting the tables
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processTables(&$asset, &$o, $prefix)
	{
		if (isset($_POST['tables'])) {
			$tables = $_POST['tables'];
			$tables = implode(', ', $tables);
			$old_tables = $asset->attr('tables');
			if (strcasecmp($old_tables, $tables) != 0) {
				$asset->setAttrValue('tables', $tables);
				return true;
			}
		}
		return false;

	}//end processTables()


	/**
	* Function to get all the tables of the database
	*
	* @param object	&$asset	the Data Source asset
	*
	* @return array
	* @access public
	*/
	function _getAllTables(&$asset)
	{
		$dsn = $asset->attr('dsn');
		$type = $asset->attr('db_type');
		$db = null;
		$tables = Array();
		$conn = null;
		if (!empty($dsn)) {
			if (empty($this->conn)) {
				$conn = $asset->connectToDB();
			} else {
				$conn = $this->conn;
			}
			if ($type == 'postgres') {
				$tables =& $conn->getAll('select relname from pg_stat_user_tables order by relname');
				$assert_result = assert_valid_db_result($tables, 'Query execution failed', false, false);
				if (!$assert_result) return Array();
				foreach ($tables as $key =>$row) {
					$tables[$row['relname']] = $row['relname'];
					unset($tables[$key]);
				}
			} else if ($type == 'mysql') {
				$tables =& $conn->getAll('show tables');
				$assert_result = assert_valid_db_result($tables, 'Query execution failed', false, false);
				if (!$assert_result) return Array();
				foreach ($tables as $key => $row) {
					foreach ($row as $index => $name) {
						$tables[$name] = $name;
					}
					unset($tables[$key]);
				}
			}
		}
		return $tables;

	}//end _getAllTables()


//--        FIELDS        --//


	/**
	* Paints the interface for entering and processing the fields
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintFields(&$asset, &$o, $prefix)
	{
		$fields = Array();
		$tables = Array();
		$selected = Array();
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		if (isset($_POST['tables'])) {
			$tables = $_POST['tables'];
		} else {
			$tables = $asset->attr('tables');
			$tables = explode(', ', $tables);
		}
		$fields = $this->_getFields($tables, $asset);
		if (isset($fields['select'])) {
			unset($fields['select']);
		}
		if (isset($_POST['fields'])) {
			foreach ($_POST['fields'] as $table => $field_array) {
				foreach ($field_array as $field) {
					$field_name = substr($field, strpos($field, '.') + 1);
					$selected[] = $field;
				}
			}
		} else {
			$selected = explode(', ', $asset->attr('fields'));
		}
		if (!empty($fields)) {
			if ($wa) {
				?>
				<table class="sq-backend-table">
					<tr>
						<?php
						foreach ($tables as $table) {
							?><td><b><?php echo $table;?></b></td>
							<td>&nbsp;</td>
						<?php
						}
						?>
					</tr>
					<tr>
						<?php
						foreach ($fields as $table => $field_array) {
							?>
							<td>
								<?php
								if (!empty($field_array)) {
									combo_box('fields['.$table.']', $field_array, true, $selected, 6);
								} else {
									echo 'Table Empty! No fields to select';
									unset($tables[array_search($table, $tables)]);
									$asset->setAttrValue('tables', implode(', ', $tables));
								}
								?>
							</td>
							<td> &nbsp; </td>
						<?php
						}
						?>
					</tr>
				</table>
				<?php
			} else {
				if (!empty($selected)) {
					if ($selected[0] == '*') {
						unset($selected[0]);
						if (!empty($fields)) {
							foreach ($fields as $table => $field_array) {
								foreach ($field_array as $key => $field) {
									$selected[] = $field;
								}
							}
						}
					}
					echo implode(', ', $selected);
				} else {
					echo '<b'.translate('no_selected_fields').'</b>';
				}
			}
		} else {
			echo '<b'.translate('no_selected_tables').'</b>';
		}
		echo ('&nbsp;');
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintFields()


	/**
	* Processes the interface for processing the fields
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFields(&$asset, &$o, $prefix)
	{
		$string_fields = null;
		if (isset($_POST['fields'])) {
			$fields = $_POST['fields'];
			foreach ($fields as $table => $fields_array) {
				$string_fields .= implode(', ',$fields_array).', ';

			}
			$string_fields = substr($string_fields, 0, strlen($string_fields)-2);
		} else {
			$string_fields = $asset->attr('fields');
			if (empty($string_fields)) $string_fields = '*';
		}
		$old_string_fields = $asset->attr('fields');
		if (strcasecmp($old_string_fields, $string_fields) != 0) {
			$asset->setAttrValue('fields', $string_fields);
			return true;
		} else {
			return false;
		}

	}//end processFields()


	/**
	* Function that takes the tables names and returns associative array of column names with table names as key
	*
	* @param array	$tables	the array of table names
	* @param object	&$asset	the data_source_db asset
	*
	* @return array
	* @access public
	*/
	function _getFields($tables, &$asset)
	{
		$conn = $asset->connectToDB();
		$fields = Array();
		if (!is_array($tables)) $tables = Array($tables);
		$db_type = $asset->attr('db_type');
		$query = '';
		foreach ($tables as $key =>  $table) {
			if ($table == 'select') {
				unset($tables[$key]);
				continue;
			}
			if ($db_type == 'mysql') {
				$query = 'show columns from '.$table;
			} else {
				if ($db_type == 'postgres') {
					$query = "SELECT column_name FROM information_schema.columns where table_name = '$table'";
				}
			}
			$result =& $conn->getAll($query);
			$assert_result = assert_valid_db_result($tables, 'Query execution failed', false, false);
			if (!$assert_result) return Array();
			if (!empty($result)) {
				foreach ($result as $record) {
					if ($db_type == 'mysql') {
						$fields[$table][$table.'.'.$record['Field']] = $record['Field'];
					} else {
						$fields[$table][$table.'.'.$record['column_name']] = $record['column_name'];
					}
				}
				if (empty($record)) $fields[$table] = '';
			}
		}
		return $fields;

	}//end _getFields()


	/**
	* Paints the interface for entering and processing the custom fields
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintCustomFields(&$asset, &$o, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}

		$custom_fields = $asset->attr('custom_fields');
		if ($wa) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td>Custom Field</td>
					<td>Alias</td>
					<td>Delete?</td>
				</tr>
				<?php
				$i = 0;
				if (!empty($custom_fields)) {
					foreach ($custom_fields as $field) {
					?>
					<tr>
						<td>
							<?php
							if ($wa) {
								text_box('custom_field['.$i.'][field]', $field['field'], 60);
							} else {
								echo $field;
							}
							?>
						</td>
						<td>
							<?php
							if ($wa) {
								text_box('custom_field['.$i.'][alias]', $field['alias'], 20);
							} else {
								echo $field['alias'];
							}
							?>
						</td>
						<?php
							if ($wa) {
								echo '<td>';
								check_box('custom_field['.$i.'][delete]', '1');
								echo '</td>';
							}
						?>
					</tr>
					<?php
					$i++;
					}
				}
				?>
				<tr>
					<td>
					<?php text_box('custom_field[new][field]', '', 60); ?>
					</td>
					<td>
					<?php text_box('custom_field[new][alias]', '', 20); ?>
					</td>
					<td>
					</td>
				</tr>
			</table>
			<?php
			hidden_field('custom_field_submit', '1');
		} else {
			if (!empty($custom_fields)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td>Custom Field</td>
					<td>Alias</td>
				</tr>
				<?php
				foreach ($custom_fields as $field) {
					echo '<tr><td>';
					echo $field['field'];
					echo '</td><td>';
					echo $field['alias'];
					echo '</td></tr>';
				}
				?>
			</table>
			<?php
			}
		}
		return $wa;

	}//end paintCustomFields()


	/**
	* Processes the interface for processing the custom fields
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processCustomFields(&$asset, &$o, $prefix)
	{
		$mode = $asset->attr('mode');
		$process = isset($_POST['custom_field_submit']);
		if ($process) {
			$custom_fields = $asset->attr('custom_fields');
			$i = 0;
			$deleted = false;
			if (!empty($custom_fields)) {
				$post_fields = $_POST['custom_field'];
				// update
				foreach ($custom_fields as $key => $field) {
					if ($field['field'] != $post_fields[$key]['field']) {
						$custom_fields[$key]['field'] = $post_fields[$key]['field'];
					}
					if ($field['alias'] != $post_fields[$key]['alias']) {
						$custom_fields[$key]['alias'] = $post_fields[$key]['alias'];
					}
					$i++;
				}
				// delete
				foreach ($post_fields as $key => $field) {
					if (isset($field['delete']) && $field['delete'] == '1' || empty($field['field'])) {
						if (!$deleted) $deleted = true;
						unset($custom_fields[$key]);
					}
				}
			}
			// add a new one
			if (isset($_POST['custom_field']['new']) && !empty($_POST['custom_field']['new']['field'])) {
				$custom_fields[$i]['field'] = $_POST['custom_field']['new']['field'];
				$custom_fields[$i]['alias'] = $_POST['custom_field']['new']['alias'];
			}

			// reset the key if any fields have been deleted
			if ($deleted) {
				reset($custom_fields);
				$new_custom_fields = Array();
				if (!empty($custom_fields)) {
					$i = 0;
					foreach ($custom_fields as $field) {
						$new_custom_fields[$i] = $field;
						$i++;
					}
				}
				$asset->setAttrValue('custom_fields', $new_custom_fields);
			} else {
				$asset->setAttrValue('custom_fields', $custom_fields);
			}
		}

		return $process;

	}//end processCustomFields()


//--        JOIN        --//


	/**
	* Paints the interface for choosng to enter a new join
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintNewJoin(&$asset, &$o, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$tables = $asset->attr('tables');
		if (($wa) && !empty($tables)) {
			$tables = explode(', ', $tables);
			foreach ($tables as $index => $table) {
				$tables[$table] = $table;
				unset($tables[$index]);
			}
			$tables['select'] = translate('select');
			?>
			<table class="sq-backend-table">
				<tr>
					<?php
					for ($i=0; $i<2; $i++) {
						?>
						<td><?php combo_box('join_tables['.$i.']', $tables, false, 'select');?></td>
					<?php
					}
				?>
				</tr>
			</table>
		<?php
			return true;
		} else {
			return false;
		}

	}//end paintNewJoin()


	/**
	* processes the interface for choosng to enter a new join
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processNewJoin(&$asset, &$o, $prefix)
	{
		return false;

	}//end processNewJoin()


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintJoins(&$asset, &$o, $prefix)
	{
		$type_joins = Array(
						'left_join'		=> translate('left_join'),
						'right_join'	=> translate('right_join'),
						'inner_join'	=> translate('inner_join'),
						'cross_join'	=> translate('cross_join'),
						'outer_join'	=> translate('outer_join'),
					  );
		$joins = $asset->attr('join');
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$join_tables = Array();
		$new_tables = Array();
		if (isset($_POST['join_tables'])) {
			$new_tables = $_POST['join_tables'];
			foreach ($new_tables as $index => $table) {
				if ($table == 'select') {
					unset($new_tables[$index]);
					continue;
				}
				$join_tables[] = $table;
			}
		}
		foreach ($joins as $join ) {
			foreach ($join as $key => $column) {
				if (strcasecmp($key, 'type') != 0) {
					$join_tables[] = substr($column, 0, strpos($column, '.'));
				}
			}
		}
		$join_tables = array_unique($join_tables);
		$all_fields = $this->_getFields($join_tables, $asset);
		if (!empty($all_fields)) {
			?>
			<table class="sq-backend-table">
				<tr>
					<td><b><?php echo translate('table');?> 1</b></td>
					<td><b><?php echo translate('column');?> 1</b></td>
					<td><b><?php echo translate('table');?> 2</b></td>
					<td><b><?php echo translate('column');?> 2</b></td>
					<td><b><?php echo translate('join_type');?></b></td>
					<?php
					if ($wa) {
						?>
						<td><b><?php echo translate('delete');?></b></td>
					<?php
					}
				?>
				</tr>
				<?php
				$i = 0;
				if (!empty($joins)) {
					foreach ($joins as $join) {
						$j = 0;
						?>
						<tr>
						<?php
						foreach ($join as $index => $column) {
							$table = null;
							if (strcasecmp($index, 'type') != 0) {
								$table = substr($column, 0, strpos($column, '.'));
								?>
								<td><?php echo $table;?></td>
								<td>
									<?php
									if ($wa) {
										combo_box('join_field['.$i.']['.$j.']', $all_fields[$table], false, $column);
									} else {
										echo $column;
									}
									?>
								</td>
							<?php
							} else {
								?>
								<td>
									<?php
									if ($wa) {
										combo_box('join_field['.$i.'][type]', $type_joins, false, $column);
									} else {
										echo $column;
									}
									?>
								</td>
							<?php
							}
							?>
							<?php
							$j++;
						}
						if ($wa) {
							?>
							<td><?php check_box('join_field['.$i.'][delete]','delete', false);?></td>
						<?php
						}
						?>
						</tr>
					<?php
					$i++;
					}
				}
				//painting new join here
				if (!empty($new_tables) && count($new_tables) == 2) {
					$j = 0;
					?>
					<tr>
						<?php
						foreach ($new_tables as $table) {
							?>
							<td><?php echo $table; ?></td>
							<td><?php combo_box('join_field['.$i.']['.$j.']', $all_fields[$table], false, '');?></td>
						<?php
						$j++;
						}
						?>
						<td><?php combo_box('join_field['.$i.'][type]', $type_joins, false, '');?></td>
						<?php
						if ($wa) {
							?>
							<td><?php check_box('join_field['.$i.']['.$table.'][delete]', 'delete', false, '');?></td>
						<?php
						}
						?>
					</tr>

				<?php
				}

			?>
			</table>
		<?php
		}
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintJoins()


	/**
	* Paints the interface for entering Data Source details
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function processJoins(&$asset, &$o, $prefix)
	{
		if (isset($_POST['join_field'])) {
			$joins = $_POST['join_field'];
			foreach ($joins as $index => $join) {
				if (isset($join['delete'])) {
					unset($joins[$index]);
					continue;
				}
				if (count($join) !=3) {
					unset($joins[$index]);
					continue;
				}
			}
			$old_join = $asset->attr('join');
			if ($old_join != $joins) {
				$asset->setAttrValue('join', $joins);
				return true;
			}
		}
		return false;

	}//end processJoins()


	/**
	* Function that assigns null and empty array to the query attributes of the asset
	*
	* @param object	&$asset	the DB Data source asset
	* @param string	$mode	the mdoe in which the query buidler is
	*
	* @return boolean
	* @access public
	*/
	function _resetAttributes(&$asset, $mode='')
	{
		if ($mode != 'edit') {
			$asset->setAttrValue('sql', null);
			$asset->setAttrValue('shadow_name', null);
			$asset->setAttrValue('reset_edit', false);
		} else {
			$asset->setAttrValue('reset_edit', true);
		}
		$asset->setAttrValue('where', Array());
		$asset->setAttrValue('join', Array());
		$asset->setAttrValue('group_by', Array());
		$asset->setAttrValue('order_by', Array());
		$asset->setAttrValue('having', Array());
		$asset->setAttrValue('fields', null);
		$asset->setAttrValue('tables', null);
		$asset->setAttrValue('aggregates', Array());
		$asset->setAttrValue('limit', 0);

	}//end _resetAttributes()


//--        AGGREGATE        --//


	/**
	* Paints the interface for entering the name of the shadow asset
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintAggregateFunctions(&$asset, &$o, $prefix)
	{
		$i = 0;
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$old_aggregates = $asset->attr('aggregates');
		$aggregates = Array(
						'AVG'		=> 'AVG(column)',
						'COUNT(*)'	=> 'COUNT(*)',
						'COUNT'		=> 'COUNT(column)',
						'SUM'		=> 'SUM(column)',
						'MAX'		=> 'MAX(column)',
						'MIN'		=> 'MIN(column)',
					  );
		if ($wa) {
			$tables = $asset->attr('tables');
			$tables = explode(', ', $tables);
			$fields = Array();
			if (!empty($tables)) {
				$fields = $this->_getFields($tables, $asset);
			}
			if (!empty($fields)) {
			?>
				<table class="sq-backend-table">
					<tr>
						<td><b><?php echo translate('select_aggregate_func');?></b></td>
						<td><b><?php echo translate('select_table');?></b></td>
					</tr>
					<tr>
						<td><?php combo_box('aggregate_func', $aggregates, false, '', 6)?> </td>
						<td><?php combo_box('aggregate_table', $tables, false, '', 6);?></td>
					</tr>
				</table>
				&nbsp;

			<?php
			}
			if (!empty($old_aggregates) || isset($_POST['aggregate_func'])) {
				?>
				<table class="sq-backend-table">
					<tr>
						<td><b><?php echo translate('aggregate_function');?></b></td>
						<td><b><?php echo translate('aggregate_field');?></b></td>
						<td><b><?php echo translate('alias');?></b></td>
						<td><b><?php echo translate('delete');?></b></td>
					</tr>
				<?php
			}
			if (!empty($old_aggregates)) {
				?>
					<?php
					foreach ($old_aggregates as $key => $aggregate) {
						$aggr_table = null;
						if ($aggregate['function'] != 'COUNT(*)') {
							$aggr_table = substr($aggregate['field'], 0, strpos($aggregate['field'], '.'));
						}
						?>
						<tr>
							<td><?php combo_box('aggregate['.$i.'][function]', $aggregates, false, $aggregate['function']); ?></td>
							<?php
							if ($aggregate['function'] != 'COUNT(*)') {
								?>
								<td><?php combo_box('aggregate['.$i.'][field]', $fields[$aggr_table], false, $aggregate['field']);?></td>
							<?php
							} else {
								?>
								<td>&nbsp;</td>
							<?php
							}
							?>
							<td><?php text_box('aggregate['.$i.'][alias]', !empty($aggregate['alias'])?$aggregate['alias']:'');?></td>
							<td><?php check_box('aggregate['.$i.'][delete]', 'delete', false, '');?></td>
						</tr>
						<?php
						$i++;
					}
					if (!isset($_POST['aggregate_func'])) echo '</table>';
					?>
				<?php
			}
			if (isset($_POST['aggregate_func'])) {
				$function = $_POST['aggregate_func'];
				$new_table= null;
				isset($_POST['aggregate_table'])? $new_table = $tables[$_POST['aggregate_table']] : $new_table = null;
				if (($function != 'COUNT(*)') && empty($new_table)) {
					return false;
				}
				?>
				<tr>
					<td><?php combo_box('aggregate['.$i.'][function]', $aggregates, false, $function); ?></td>
					<?php
					if ($function != 'COUNT(*)') {
						?>
						<td><?php combo_box('aggregate['.$i.'][field]', $fields[$new_table], false, '');?></td>
					<?php
					} else {
					?>
					<td> &nbsp; </td>
					<?php
					}
					?>
					<td><?php text_box('aggregate['.$i.'][alias]', '');?></td>
					<td><?php check_box('aggregate['.$i.'][delete]', 'delete', false, '');?></td>
				</tr>
				</table>
				<?php
			}
		} else {
			if (!empty($old_aggregates)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<td><b><?php echo translate('aggregate_function');?></b></td>
						<td><b><?php echo translate('aggregate_field');?></b></td>
						<td><b><?php echo translate('alias');?></b></td>
					</tr>
					<?php
					foreach ($old_aggregates as $key => $aggregate) {
						?>
						<tr>
							<td><?php echo $aggregate['function']; ?></td>
							<td><?php echo(isset($aggregate['field'])? $aggregate['field']: '&nbsp;');?></td>
							<td><?php echo(!empty($aggregate['alias'])? $aggregate['alias']: 'No Alias Set');?></td>
						</tr>
						<?php
					}
				?>
				</table>
			<?php
			}
		}

		echo ('&nbsp;');
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintAggregateFunctions()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processAggregateFunctions(&$asset, &$o, $prefix)
	{
		if (isset($_POST['aggregate'])) {
			$aggregates = $_POST['aggregate'];
			foreach ($aggregates as $key => $aggregate) {
				if (isset($aggregate['delete'])) {
					unset($aggregates[$key]);
				}
			}
			$old_aggr = $asset->attr('aggregates');
			if ($old_aggr != $aggregates) {
				$asset->setAttrValue('aggregates', $aggregates);
				return true;
			} else {
				return false;
			}
		}
		return false;

	}//end processAggregateFunctions()


//--        WHERE HAVING        --//


	/**
	* Paints the interface for entering and changing the where clause
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintWhereHaving(&$asset, &$o, $prefix)
	{
		$tables = explode(', ', $asset->attr('tables'));
		$operators = Array('=', '<>', '<', '>', '<=', '>=', 'LIKE');
		$conditions = Array('Relation', 'Between', 'In', 'Is Null', 'Like', 'Other');
		$clauses = Array('where', 'having');
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$multi_clauses = Array('none' => translate('none'), 'AND' => translate('and'), 'OR' => translate('or'));
		$fields = $this->_getFields($tables, $asset);
		$tables['select'] = translate('select_clause_table');

		if ($wa) {
			combo_box('clause_table', $tables, false, 'select');
			combo_box('conditional_panel', Array ('select' => translate('select_clause_type'), 'where' => translate('where'), 'having' => translate('having')), false, 'select');
		}
		foreach ($clauses as $clause) {
			$i = 0;
			$entered_clauses = $asset->attr($clause);
			if (!empty($entered_clauses)) {
				echo ('<h4>'.translate($clause).'</h4>');
			}
			?>
			<table class="sq-backend-table">
				<?php
				foreach ($entered_clauses as $index => $clause_value) {
					$condition = $clause_value['condition'];
					$table = substr($clause_value['field'], 0, strpos($clause_value['field'], '.'));
					if ($index == 0) {
						?>
						<tr>
							<td><b><?php echo translate('field');?></b></td>
							<td><b><?php echo translate('condition');?></b></td>
							<td><b><?php echo translate('operator_or_min');?></b></td>
							<td><b><?php echo translate('value_or_max');?></b></td>
							<td><b><?php echo translate('multiclause_join_op');?></b></td>
							<?php
							if ($wa) {
								?>
								<td><?php echo translate('delete');?></td>
							<?php
							}
						?>
						</tr>
					<?php
					}
					if ($wa) {
						?>
						<tr>
							<td><?php combo_box($clause.'['.$index.'][field]', $fields[$table], false, $clause_value['field']);?></td>
							<td><?php combo_box($clause.'['.$index.'][condition]', $conditions, false, array_search($clause_value['condition'], $conditions));?></td>
							<?php
							switch ($condition) {
								case 'Relation':
									?>
									<td><?php combo_box($clause.'['.$index.'][operator]', $operators, false, isset($clause_value['operator'])?array_search($clause_value['operator'], $operators):'');?></td>
									<td><?php text_box($clause.'['.$index.'][value]', isset($clause_value['value'])?$clause_value['value']:''); ?> </td>
								<?php
								break;
								case 'Between':
									?>
									<td><?php text_box($clause.'['.$index.'][min_value]', isset($clause_value['min_value'])?$clause_value['min_value']:''); ?> </td>
									<td><?php text_box($clause.'['.$index.'][max_value]', isset($clause_value['max_value'])?$clause_value['max_value']:''); ?> </td>
								<?php
								break;

								case 'Is Null':
									?>
									<td>&nbsp;</td>
									<td>&nbsp;</td>
								<?php
								break;

								case 'In':

								case 'Like':

								case 'Other':
									?>
									<td>&nbsp;</td>
									<td><?php text_box($clause.'['.$index.'][value]', isset($clause_value['value'])?$clause_value['value']:''); ?> </td>
								<?php
								break;
							}
							?>
							<td> <?php combo_box($clause.'['.$index.'][multi]', $multi_clauses, false, isset($clause_value['multi'])?$clause_value['multi']:'none');?></td>
							<td><?php check_box($clause.'['.$index.'][delete]', 'delete', false);?></td>
						</tr>
					<?php
					} else {
						?>
						<tr>
							<td><?php echo $clause_value['field'];?></td>
							<td><?php echo $clause_value['condition'];?></td>
							<td>
								<?php
								if (isset($clause_value['operator'])) {
									echo $clause_value['operator'];
								} else if (isset($clause_value['min_value'])) {
									echo $clause_value['min_value'];
								} else {
									echo '&nbsp;';
								}
								?>
							</td>
							<td>
								<?php
								if (isset($clause_value['value'])) {
									echo $clause_value['value'];
								} else if (isset($clause_value['max_value'])) {
									echo $clause_value['max_value'];
								} else {
									echo '&nbsp;';
								}
								?>
							</td>
							<td><?php echo $clause_value['multi'];?></td>
						</tr>
					<?php
					}
					$i++;
				}//end of traversing the enetered clauses
				?>
			</table>
		<?php
		}
		if (isset($_POST['clause_table']) && isset($_POST['conditional_panel'])) {
			$clause = $_POST['conditional_panel'];
			$table = $tables[$_POST['clause_table']];
			if (($table != 'select') && ($clause != 'select')) {
				$entered_clauses = $asset->attr($clause);
				$index = count($entered_clauses);
				?>
				<h4><?php echo translate('new').' '.$clause.' '.translate('clause');?></h4>
				<table class="sq-backend-table">
					<tr>
						<td><b><?php echo translate('field');?></b></td>
						<td><b><?php echo translate('condition');?></b></td>
						<?php
						if ($wa) {
							?>
							<td><?php echo translate('delete');?></td>
						<?php
						}
						?>
					</tr>
					<tr>
						<td><?php combo_box($clause.'['.$index.'][field]', $fields[$table], false, '');?></td>
						<td><?php combo_box($clause.'['.$index.'][condition]', $conditions, false, '');?></td>
						<?php
						if ($wa) {
							?>
							<td><?php check_box($clause.'['.$index.'][delete]', 'delete', false);?></td>
						<?php
						}
						hidden_field($clause.'['.$index.'][operator]', ' ');
						hidden_field($clause.'['.$index.'][value]', '');
						hidden_field($clause.'['.$index.'][multi]', '');

						?>
					</tr>
				</table>
			<?php
			}
		}
		echo '&nbsp;';
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintWhereHaving()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processWhereHaving(&$asset, &$o, $prefix)
	{
		$clauses = Array('where', 'having');
		$operators = Array('=', '<>', '<', '>', '<=', '>=', 'LIKE');
		$conditions = Array('Relation', 'Between', 'In', 'Is Null', 'Like', 'Other');
		$flag = 0;
		foreach ($clauses as $clause) {
			if (isset($_POST[$clause])) {
				$entered_clauses = $_POST[$clause];
				foreach ($entered_clauses as $key => $clause_value) {
					$entered_clauses[$key]['condition'] = $conditions[$clause_value['condition']];
					if (isset($clause_value['operator'])) {
						$entered_clauses[$key]['operator'] = isset($operators[$clause_value['operator']])? $operators[$clause_value['operator']]: '';
					}
					if (isset($clause_value['delete'])) {
						unset($entered_clauses[$key]);
					}
				}
				$old_value = $asset->attr($clause);
				if ($old_value != $entered_clauses) {
					$asset->setAttrValue($clause, $entered_clauses);
					$flag = 1;
				}
			}
		}
		if ($flag == 1) {
			return true;
		} else {
			return false;
		}

	}//end processWhereHaving()


//--        GROUP BY        --//


	/**
	* Paints the interface for entering and changing the where clause
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintGroupbyOrderby(&$asset, &$o, $prefix)
	{
		$fields = $asset->attr('fields');
		if ($fields != '*') {
			$fields = explode(', ', $fields);
			foreach ($fields as $index => $field) {
				$fields[$field] = $field;
				unset($fields[$index]);
			}
		} else {
			$tables = explode(', ', $asset->attr('tables'));
			$fields = $this->_getFields($tables, $asset);
			foreach ($fields as $table => $field_array) {
				foreach ($field_array as $key => $field) {
					$fields[$key] = $key;
				}
				unset($fields[$table]);
			}
		}
		$aggregates = $asset->attr('aggregates');
		if (!empty($aggregates)) {
			foreach ($aggregates as $aggregate) {
				if (isset($aggregate['alias'])) {
					$fields[$aggregate['alias']] = $aggregate['alias'];
				} else {
					$fields[$aggregate['function']] = $aggregate['function'];
				}
			}
		}
		$custom_fields = $asset->attr('custom_fields');
		if (!empty($custom_fields)) {
			foreach ($custom_fields as $field) {
				$fields[$field['alias']] = $field['alias'];
			}
		}

		$priority = Array();
		$order = Array('ASC' => translate('order_asc'), 'DESC' => translate('order_desc'));
		for ($i = 1; $i <= count($fields); $i++) {
			$priority[$i] = $i;
		}
		$clauses = Array('order_by', 'group_by');
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$wa = true;
		} else {
			$wa = false;
		}
		$fields['select'] = translate('select_clause_field');
		if ($wa) {
			combo_box('by_field', $fields, false, 'select');
			combo_box('by_panel', Array ('select' => translate('select_clause_type'), 'order_by' => translate('order_by'), 'group_by' => translate('group_by')), false, 'select');
		}
		foreach ($clauses as $clause) {
			$i = 0;
			$entered_clauses = $asset->attr($clause);
			if (!empty($entered_clauses)) {
				echo ('<h4>'.ucwords(str_replace('_', ' ',$clause)).'</h4>');
			}
			?>
			<table class="sq-backend-table">
				<?php
				foreach ($entered_clauses as $key => $clause_value) {
					if ($key == 0) {
						?>
						<tr>
							<td><?php echo translate('field');?></td>
							<td><?php echo translate('priority');?></td>
							<?php
							if ($clause == 'order_by') {
								?>
								<td><?php echo translate('order');?></td>
							<?php
							}
							if ($wa) {
								?>
								<td><?php echo translate('delete');?></td>
							<?php
							}
							?>
						</tr>
					<?php
					}
					if ($wa) {
						?>
						<tr>
							<td><?php combo_box($clause.'['.$i.'][field]', $fields, false, $clause_value['field']);?></td>
							<td><?php combo_box($clause.'['.$i.'][priority]', $priority, false, $clause_value['priority']);?></td>
							<?php
							if ($clause == 'order_by') {
								?>
								<td><?php combo_box($clause.'['.$i.'][order]', $order, false, $clause_value['order']);?></td>
							<?php
							}
							?>
							<td> <?php check_box($clause.'['.$i.'][delete]', 'delete', false);?> </td>
						</tr>
						<?php
					} else {
						?>
						<tr>
							<td><?php echo $clause_value['field'];?></td>
							<td><?php echo $clause_value['priority'];?></td>
							<?php
							if ($clause == 'order_by') {
								?>
								<td><?php echo $clause_value['order'];?></td>
							<?php
							}
					}
					$i++;
				}//end of traversing the enetered clauses
				?>
			</table>
		<?php
		}
		echo '&nbsp;';
		if ($wa) {
			return true;
		} else {
			return false;
		}

	}//end paintGroupbyOrderby()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processGroupbyOrderby(&$asset, &$o, $prefix)
	{
		$clauses = Array('order_by', 'group_by');
		$entered_clauses = Array();
		foreach ($clauses as $clause) {
			if (isset($_POST[$clause])) {
				$entered_clauses[$clause] = $_POST[$clause];
				foreach ($entered_clauses[$clause] as $key => $clause_value) {
					if (isset($clause_value['delete'])) {
						unset($entered_clauses[$clause][$key]);
					}
				}
			}
		}
		if (isset($_POST['by_field']) && isset($_POST['by_panel'])) {
			$field = $_POST['by_field'];
			$clause = $_POST['by_panel'];
			if (($field != 'select') && ($clause != 'select')) {
				!empty($entered_clauses[$clause])? $index = count($entered_clauses[$clause]): $index =0;
				$entered_clauses[$clause][$index]['field'] = $field;
				$entered_clauses[$clause][$index]['priority'] = $index;
				if ($clause == 'order_by') {
					$entered_clauses[$clause][$index]['order'] = 'ASC';
				}
			}
		}
		if (!empty($entered_clauses)) {
			foreach ($entered_clauses as $clause => $value) {
				$old_clause_value = $asset->attr($clause);
				if ($old_clause_value != $value) {
					$asset->setAttrValue($clause, $value);
				}
			}
			return true;
		}
		return false;

	}//end processGroupbyOrderby()


//--        QUERY        --//


	/**
	* Paints the interface for painting the query
	*
	* @param object	&$asset	the DB Data source asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintQuerySoFar(&$asset, &$o, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes') && ($mode == 'edit');
		if ($asset->writeAccess('attributes')) {
			$this->_queryProcessing($asset, $prefix);
		}
		$sql = $asset->attr('sql');

		switch ($mode) {
			case 'edit':
				//if we are currently in edit mode
				hidden_field($prefix.'_from_edit', '1');
				break;
			case 'execute':
				//we are currently in execute mode but we came here from edit mode
				if (isset($_POST[$prefix.'_from_edit']) && ($_POST[$prefix.'_from_edit']) == 1) {
					hidden_field($prefix.'_from_edit', '1');
				}
				break;
		}

		if ($wa) {
			$sql = str_replace('<br />', ' ', $sql);
			text_area($prefix.'_sql', $sql, 100, 2);
		} else {
			if (!empty($sql)) {
				echo $sql;
				hidden_field($prefix.'_sql', $sql);
			}
		}
		echo('&nbsp;');

		return $wa;

	}//end paintQuerySoFar()


	/**
	* Processes the interface for entering and reassigning the sql query
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuerySoFar(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_sql'])) {
			$asset->setAttrValue('sql', $_POST[$prefix.'_sql']);
		}
		$mode = $asset->attr('mode');
		switch ($mode) {
			case 'edit':
				$this->_resetAttributes($asset, 'edit');
			break;
			case 'builder':
				$asset->setAttrValue('reset_edit', false);
			break;
		}
		return true;

	}//end processQuerySoFar()


	/**
	* Processes the current sql by taking all the attributes
	*
	* @param object	&$asset	the search page asset
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function _queryProcessing(&$asset, $prefix)
	{
		$reset_edit = $asset->attr('reset_edit');
		if ($reset_edit) {
			$sql = null;
			if (isset($_POST[$prefix.'_sql'])) {
				$sql = $_POST[$prefix.'_sql'];
			} else {
				$sql = $asset->attr('sql');
			}
			if (empty($sql)) {
				$res = $this->_resetAttributes($asset, 'reset');
			} else {
				$res = $this->_resetAttributes($asset, 'edit');
			}
			$asset->setAttrValue('sql', $sql);
			return true;
		}

		//get the tables and the fields attribute values
		$tables = $asset->attr('tables');
		$fields = $asset->attr('fields');
		$custom_fields = $asset->attr('custom_fields');
		$aggregates = $asset->attr('aggregates');
		$joins = $asset->attr('join');

		//start query
		$sql = '';
		if (!empty($tables)) {
			$sql = 'SELECT ';
			if (isset($_POST['show']) && $fields != '*') {
				$sql .= ' '.strtoupper($_POST['show']);
			}
			if (($fields == '*') & !empty($aggregates)) {
				$temp_fields = $this->_getFields(explode(', ', $tables), $asset);
				$count = 0;
				foreach ($temp_fields as $fields_array) {
					if ($count !=0 ) $sql .= ' , ';
					$sql .= implode(', ', array_keys($fields_array));
					$count++;
				}
			} else {
				$sql .= ' '.$fields;
			}
			if (!empty($aggregates)) {
				foreach ($aggregates as $key => $aggregate) {
					$sql .= ', '.$aggregate['function'];
					if ($aggregate['function'] != 'COUNT(*)') {
						$sql .= '('.$aggregate['field'].')';
					}
					if (!empty($aggregate['alias'])) {
						$sql .= ' AS '.$aggregate['alias'];
					}
				}
			}
			if (!empty($custom_fields)) {
				foreach ($custom_fields as $key => $field) {
					$sql .= ', '.$field['field'].' AS '.$field['alias'];
				}
			}
			$sql .= '<br />FROM ';

			//if the user has joins then we cannot add the tables to the sql normally
			//we have to check on the tables used for the joins and add the remaining normally and add then add the joins!

			if (!empty($joins)) {
				$tab_array = explode(', ', $tables);
				foreach ($joins as $index => $join) {
					$table = Array();
					foreach ($join as $key => $column) {
						if (strcasecmp($key, 'type') != 0) {
							$temp = substr($column, 0, strpos($column, '.'));
							$table[] = $temp;
							$i = array_search($temp, $tab_array);
							if ($i !== false) unset($tab_array[$i]);
						} else {
							$table['type'] = ucwords(str_replace('_', ' ', $column));
						}
					}
					if ($index != 0) {
						$sql .= ' ,<br />';
					} else {

						$sql .= '<br />'.implode(', ', $tab_array);
					}
					$sql .= empty($tab_array)? ' ':', ';
					$sql .= $table[0].' '.$table['type'].' '.$table[1].' ON '.$join[0].' = '.$join[1];
				}
			} else {
				$sql .= ' '.$tables;
			}

			$bys = Array('where', 'group_by', 'having', 'order_by');
			$flag_group = 0;

			//get each attribute value and assign it to the query
			foreach ($bys as $key) {
				$value = $asset->attr($key);
				if (empty($value)) continue;

				//you cannot have 'having' without group by
				//flag_goup os set to 1 once we have a group by clause in the sql
				if (($key == 'having') && ($flag_group == 0)) {
					continue;
				}

				$sql .= '<br />'.strtoupper(str_replace('_', ' ' ,$key));

				if ($key == 'group_by') {
					$groups = Array();
					$flag_group = 1;
					$priority = Array();
					foreach ($value as $index => $val) {
						$groups[$val['field']] = $val['priority'];
						$priority[] = $val['priority'];
					}
					$result = asort($groups);
					/*
					if ($fields != '*') {
						$fields = explode(', ', $fields);
					} else {
						$fields_get = $this->_getFields(explode(', ', $tables), $asset);
						foreach ($fields_get as $table => $fields_array) {
							$fields = array_merge($fields, array_keys($fields_array));
						}

					}
					if (count($groups) < count($fields)) {
						foreach ($fields as $field) {
							$last_priority = end($groups);
							$keys = array_keys($groups);
							if (!in_array($field, array_keys($groups)) && $field != '*') {
								$groups[$field] = $last_priority + 1;
							}
						}
					}*/
					foreach ($groups as $field => $priority) {
						$sql .= '<br />'.$field.',';
					}
					$sql = substr($sql, 0, strlen($sql) - 1);
					$this->group_by = $groups;
					continue;
				}

				if ($key == 'where'|| $key == 'having') {
					foreach ($value as $index => $val) {
						$sql .= '<br />'.$val['field'].' ';
						!empty($val['condition']) && !in_array($val['condition'], Array('Relation', 'Other'))? $sql .= $val['condition'] : $sql .= '';
						isset($val['operator'])? $sql .= ' '.$val['operator'] : $sql .= '';
						if (trim($val['value']) != '') {
							$sql .= ' '.$val['value'];
						} else {
							if (trim($val['min_value']) != '' && trim($val['max_value']) != '') {
								$sql .=' '.$val['min_value'].' AND '.$val['max_value'];
							}
						}
						$val['multi'] != 'none'? $sql .= ' '.$val['multi']: $sql .= '';
					}
					continue;
				}

				//key is order_by
				$new_value = Array();
				foreach ($value as $index => $val) {
					//if ($index != 0) $sql .= ',';
					$new_value[$val['priority']] = $val;
				}
				$temp = Array();
				foreach ($new_value as $key => $val) {
					$temp[$key] = $key;
				}
				$new_one = asort($temp);
				foreach ($temp as $key) {
					$val = $new_value[$key];
					if ($key != 1) $sql .= ', ';
					$sql .= '<br />'.$val['field'].' '.$val['order'];
				}

			}//end for
			$limit = $asset->attr('limit');
			if (!empty($limit)) $sql .= ' LIMIT '.$limit;

		}//end of if where checking if tables is not empty
		$old_sql = $asset->attr('sql');
		$mode = $asset->attr('mode');
		//if the user has shifted into execute mode form edit mode then te old query has still to be retained!
		if ($mode == 'execute') {
			if (!empty($_POST['sql']) && empty($sql)) {
				return true;
			}
		}
		if (strcasecmp($old_sql, $sql) != 0) {
			$asset->setAttrValue('sql', $sql);
		}
		return false;

	}//end _queryProcessing()


//--        HELPER        --//


	/**
	* Find out if the asset is n query mode or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function notEditMode(&$asset, $prefix)
	{
		$query = $asset->attr('sql');
		if (empty($_POST['sql'])) {
			$asset->setAttrValue('edit_on', false);
			return true;
		} else {
			$mode = $asset->attr('mode');
			if ($mode == 'edit') {
				$asset->setAttrValue('edit_on', true);
			}
			$edit_on = $asset->attr('edit_on');
			if (($mode == 'edit') || (($mode == 'execute') && (!empty($query) || !empty($_POST['sql'])))) {
				$object_attr =& $asset->getAttribute('mode');
				if (!empty($object_attr) && ($edit_on)) {
					//$object_attr->_params
					unset($object_attr->_params['options']['builder']);
				}
					return true;
			} else {
				//return true;
				$tables = $asset->attr('tables');
				if (($mode == 'execute') && !empty($tables)) {
					return true;
				} else {
					if ($mode == 'builder') return true;
				}
			}
			return false;
		}

	}//end notEditMode()


	/**
	* Find out if there are any tables selected or not and if not in edit mode
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function tablesSelectedAndNotEditMode(&$asset, $prefix)
	{
		$res = $this->tablesSelected($asset, $prefix);
		$mode = $asset->attr('mode');
		$edit_on = $asset->attr('edit_on');
		if (($mode == 'execute') || ($mode == 'edit')) {
			return false;
		} else {
			if ($edit_on) {
				return false;
			} else {
				return $res;
			}
		}


	}//end tablesSelectedAndNotEditMode()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function tablesSelectedWithAggregates(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		$aggregates = $asset->attr('aggregates');
		if ((($wa) && ($mode != 'execute')) || !empty($aggregates)) {
			$tables = $asset->attr('tables');
			if (empty($tables)) {
				return false;
			} else {
				return true;
			}
		}

	}//end tablesSelectedWithAggregates()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function tablesSelectedWithWhereHaving(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		$where = $asset->attr('where');
		$having = $asset->attr('having');
		if ((($wa) && $mode != 'execute') || !empty($having) || !empty($where)) {
			$tables = $asset->attr('tables');
			if (empty($tables)) {
				return false;
			} else {
				return true;
			}
		}

	}//end tablesSelectedWithWhereHaving()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function tablesSelectedWithGroupbyOrderby(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		$order_by = $asset->attr('order_by');
		$group_by = $asset->attr('group_by');
		if ((($wa) && $mode != 'execute') || !empty($order_by) || !empty($group_by)) {
			$tables = $asset->attr('tables');
			if (empty($tables)) {
				return false;
			} else {
				return true;
			}
		}

	}//end tablesSelectedWithGroupbyOrderby()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function tablesSelected(&$asset, $prefix)
	{
		$tables = $asset->attr('tables');
		if (empty($tables)) {
			return false;
		} else {
			return true;
		}

	}//end tablesSelected()


	/**
	* If the mode is edit then the sql section is shown but if it is not edit and the query is empty then sql section is hidden
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function queryNotEmpty(&$asset, $prefix)
	{
		$query = $asset->attr('sql');
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if ($mode == 'edit') {
			return true;
		} else {
			if (empty($query)) {
				$tables = $asset->attr('tables');
				if (empty($tables)) {
					return false;
				} else {
					return true;
				}
			} else {
				return true;
			}
		}

	}//end queryNotEmpty()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function notEditModeAndTablesNotEmpty(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');

		// only if the screen moved from 'edit' to 'execute', false
		if ($mode == 'execute' && isset($_POST[$prefix.'_from_edit'])) {
			return false;
		}

		$wa = $asset->writeAccess('attributes');
		if ($mode == 'edit') {
			return false;
		} else if ($mode == 'execute') {
			$tables = $asset->attr('tables');
			$edit_on = $asset->attr('edit_on');
			if (empty($tables) && !($edit_on)) {
				return false;
			} else {
				return true;
			}
		} else {
			return true;
		}

	}//end notEditModeAndTablesNotEmpty()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function multiTablesSelected(&$asset, $prefix)
	{
		if (count(explode(', ', $asset->attr('tables'))) > 1) {
			return true;
		}
		return false;

	}//end multiTablesSelected()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	The prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function multiTablesSelectedWithWriteAccess(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			if (count(explode(', ', $asset->attr('tables'))) > 1) {
				return true;
			}
		}
		return false;

	}//end multiTablesSelectedWithWriteAccess()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function multiTablesSelectedWithJoinsOrWriteAccess(&$asset, $prefix)
	{
		$mode = $asset->attr('mode');
		$wa = $asset->writeAccess('attributes');
		if (($wa) && ($mode != 'execute')) {
			$joins = $asset->attr('join');
			//even if there are exisitng joins or if there are new joins or join changes then retunr true
			if ((isset($_POST['join_tables']) && ($_POST['join_tables'][0] != 'select')) || (!empty($joins)) || (isset($_POST['join_field']))) {
				return true;
			}
		} else {
			$joins = $asset->attr('join');
			if (!empty($joins)) return true;
		}
		return false;

	}//end multiTablesSelectedWithJoinsOrWriteAccess()


}//end class
?>
