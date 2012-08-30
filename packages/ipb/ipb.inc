<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb.inc,v 1.21 2012/08/30 00:58:55 ewang Exp $
*
*/


/**
* IPB
*
* Purpose
* To provide utility functions to extract member and group information
* from Invision Power Board database
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.21 $
* @package Fudge
* @subpackage ipb
*/
class Ipb
{

	/**
	* The reference to IPB's db connector
	*
	* @var
	*/
	var $db;

	/**
	* The list of files we need to access in IPB
	*
	* @var array
	*/
	var $needed_files = Array(
							'/conf_global.php',
							'/ips_kernel/class_db_mysql.php',
							'/ips_kernel/class_converge.php',
						);

	var $info = NULL;
	var $version = '2_0_4';
	var $registry = NULL;
	var $command = 'fetch_row';
	var $command_free = 'free_result';


	/**
	* Constructor
	*
	* @param string	$path	absolute path to Invision Power Board installation
	*/
	function Ipb($path, $version)
	{
		if (!empty($path) && !empty($version)) {
			$this->version = $version;
			$this->setup($path);
		}

	}//end constructor


//--        SET UP        --//


	/**
	* Set up the database connection
	*
	* This function includes all necessary files from IPB,
	* then establishes db connection to their database
	*
	* @param string	$path		the absolute path to Invision Power Board installation
	* @param string	$version	the version of the Invision Power Board installation
	*
	* @return boolean
	* @access public
	*/
	function setup($path)
	{
		$ipb_root_path = $path;
		$ipb_kernel_path = $ipb_root_path.'/ips_kernel';

		// Let's check to see if IPB can be found
		if (!is_dir($ipb_root_path) || !is_dir($ipb_kernel_path)) {
			return FALSE;
		}

		// Database details
		$INFO = Array();
		if (is_file($ipb_root_path.'/conf_global.php')) {
			include $ipb_root_path.'/conf_global.php';
		} else {
			return FALSE;
		}
		$this->info =& $INFO;

		// set URL
		switch ($this->version) {
			case '2_0_4' :
				if (isset($INFO['html_url'])) {
					$this->ipb_url = $INFO['html_url'];
				} else if (isset($INFO['board_url'])) {
					$this->ipb_url = $INFO['board_url'];
				}
			break;
			case '2_1_3' :
			case '2_2_1' :
			case '3_0_2' :
				$this->ipb_url = $INFO['board_url'];
			break;
		}//end switch

		$INFO['sql_driver'] = ! $INFO['sql_driver'] ? 'mysql' : strtolower($INFO['sql_driver']);

		if ($this->version != '3_0_2') {
			require_once $ipb_kernel_path.'/class_db_'.$INFO['sql_driver'].'.php';
		}//end if

		switch ($this->version) {
			case '2_0_4' :
				if (class_exists('db_driver')) {
					$this->db = new db_driver;
				} else {
					return FALSE;
				}
			break;
			case '2_1_3' :
			case '2_2_1' :
				$driver = 'db_driver_'.$INFO['sql_driver'];
				$this->db = new $driver;
			break;
			case '3_0_2' :
				require_once($ipb_root_path.'/initdata.php');
				require_once(IPS_ROOT_PATH.'/sources/base/ipsRegistry.php');
				$this->registry = ipsRegistry::instance();
				$this->registry->init();
				error_reporting(E_ALL);
				$this->db = $this->registry->DB();
				$this->command = 'fetch';
				$this->command_free = 'freeResult';
			break;
		}

		if (!defined('IN_IPB')) {
			define ('IN_IPB', TRUE);
		}//end if
		$this->db->obj['sql_database'] = $INFO['sql_database'];
		$this->db->obj['sql_user'] = $INFO['sql_user'];
		$this->db->obj['sql_pass'] = $INFO['sql_pass'];
		$this->db->obj['sql_host'] = $INFO['sql_host'];
		$this->db->obj['sql_tbl_prefix'] = $INFO['sql_tbl_prefix'];
		$this->db->obj['query_cache_file'] = $ipb_root_path.'/sources/sql/'.$INFO['sql_driver'].'_queries.php';
		if ($this->version == '3_0_2') {
			$this->db->obj['query_cache_file'] = '';
		}//end if
		$this->db->obj['use_shutdown'] = 1;

		if (is_array($this->db->connect_vars) && count($this->db->connect_vars)) {
			foreach ($this->db->connect_vars as $k => $v) {
				$this->db->connect_vars[$k] = (isset($INFO[$k]) ? $INFO[$k] : '');
			}
		}
		if ($this->db->connect()) return TRUE;

		return FALSE;

	}//end setup()


	/**
	* Return TRUE if IPB is connected
	*
	* @return boolean
	* @access public
	*/
	function isConnected()
	{
		if (!is_null($this->db)) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end isConnected()


	/**
	* Close IPB's database connection
	*
	* @return boolean
	* @access public
	*/
	function disconnect()
	{
		if (!is_null($this->db)) {
			$result = $this->db->close_db();
			$this->_assertValidResult($this->db, 'Could not close database');
			return TRUE;
		} else {
			return FALSE;
		}

	}//end disconnect()


//--        URL        --//


	/**
	* Returns IPB website URL
	*
	* @return string
	* @access public
	*/
	function getIpbURL()
	{
		return $this->ipb_url;

	}//end getIpbURL()


//--        SEARCH        --//


	/**
	* Search the database for the given shadow assetid
	* Since IPB can have same primary key for member and group,
	* ipb_bridge adds an unique prefix for different asset type.
	* For example, member has 'm_' as a prefix and 'g_' for a group.
	*
	* @param int	$assetid	the id of the shadow asset to search
	*
	* @return mixed array|boolean
	* @access public
	*/
	function search($assetid)
	{
		$tmp = explode('_', $assetid);
		$id = $tmp[1];
		$prefix = $tmp[0];

		if (empty($id) || empty($prefix)) {
			return FALSE;
		}
		if ( $prefix == 'u') {
			// ipb member
			$result = $this->getMemberGroupInfo('members', Array(), $id);
		} else if ( $prefix == 'g' ) {
			// ipb group
			$result = $this->getMemberGroupInfo('groups', Array(), $id);
		}

		if (empty($result)) return FALSE;

		return $result;

	}//end search()


//--        AUTHENTICATION        --//


	/**
	* Authenticate the user based on the username and the password
	*
	* @param object		$bridge			The current bridge
	* @param string		$username		the user name to verify
	* @param string		$password		the password to verify
	* @param boolean	$is_email_login	it's TRUE if email is used for loggin
	*
	* @return boolean
	* @access public
	*/
	function authenticateUser(&$bridge, $username, $password, $is_email_login)
	{
		switch ($this->version) {
			case '3_0_2':
				// New style of authentication
				$member_key = '';
				if ($is_email_login) {
					if (!IPSMember::checkByEmail($username)) return FALSE;
					$member_key = $username;
				} else {
					$sql = 'SELECT m.member_id FROM '.$this->db->obj['sql_tbl_prefix'].'members m
						WHERE m.name=\''.str_replace('&#039', '&#39', htmlspecialchars($username, ENT_QUOTES)).'\';';
					$member_info = $this->_getResult($sql, Array(), TRUE);
					$member_id = array_get_index($member_info, 'member_id', 0);
					if (empty($member_id)) return FALSE;
					$member_key = $member_id;
				}//end if
				if (empty($member_key)) return FALSE;
				$md5_password = md5($password);
				$status = IPSMember::authenticateMember($member_key, $md5_password);
				return $status;
			break;
			default:
				// Old method of authentication
				require_once $bridge->attr('location').'/ips_kernel/class_converge.php';
				$converge = new class_converge($this->db);

				if ($is_email_login) {
					if (!$converge->converge_check_for_member_by_email($username)) {
						return FALSE;
					}
					$converge->converge_load_member_by_email($username);
				} else {
					$sql = 'SELECT
								id	
							FROM
								'.$this->db->obj['sql_tbl_prefix'].'members
							WHERE
								name=\''.str_replace('&#039', '&#39', htmlspecialchars($username, ENT_QUOTES)).'\';
					';
					$db_result = $this->db->query($sql);
					$this->_assertValidResult($this->db, 'Could not authenticate IPB user');

					while ($row = $this->db->{$this->command}($db_result)) {
						$test[] = $row;
					}
                                                                                                                                                                $this->db->{$this->command_free}($db_result);
					if (empty($test)) return FALSE;

					@$converge->converge_load_member_by_id($test[0]['id']);
				}

				return $converge->converge_authenticate_member(md5($password));
		}//end switch

	}//end authenticateUser()


//--        GROUPS AND MEMBERS        --//


	/**
	* Returns the summary of members and groups in multi dimen array
	*
	* The returned array can take two different forms. If cols is not specified,
	* the associative array have the id of members/groups as a key. For example,
	* Also, this function return all the information of members and groups at once.
	* <pre>
	* Array(
	*	'id'	=> Array(
	*				'name'		=> value,
	*				'mgroup		=> value,
	*				...
	*				),
	* );
	* </pre>
	* If the list of wanted columns is passed, no key is specified in the returned array
	* <pre>
	* Array(
	*	0	=> Array(
	*			'id'	=> value,
	*			'name'	=> value,
	*			'mgroup	=> value,
	*			...
	*		  ),
	*	...
	* );
	* </pre>
	*
	* @param array	$cols	the list of wanted columns
	* @param int	$id		optionally the id of member/group can be specified
	*
	* @return array
	* @access public
	*/
	function getAllInfo($cols=Array(), $id=0)
	{
		// add column names
		$columns			= Array();
		$custom_columns		= FALSE;
		$table_name			= 'members';
		$table_short_name	= 'm';
		$member_id_field = $this->_getMemberIdField();
		$member_group_id_field = $this->_getMemberGroupIdField();
		$m_cols = Array($member_id_field, 'name', $member_group_id_field, 'email', 'title', 'member_login_key');
		$g_cols	= Array('g_id', 'g_title', 'g_mem_info', 'prefix', 'suffix');

		if (empty($cols)) {
			foreach ($m_cols as $c) {
				$columns[] = 'm.'.$c;
			}
			foreach ($g_cols as $c) {
				$columns[] = 'g.'.$c;
			}
		} else {
			$custom_columns = TRUE;
		}

		// build query
		$where = '';
		if ($id) {
			$where = 'WHERE '.$table_short_name.'.'.$member_id_field.'='.$id;
		}
		$join = 'LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'groups g ON (m.'.$member_group_id_field.'=g.g_id)';
		$sql = $this->_buildQuery($columns, $table_name, $table_short_name, $where, $join);

		// get the result
		$db_result = $this->db->query($sql);
		$this->_assertValidResult($this->db, 'Could not get IPB info');

		$result = Array();
		$cols = array_merge($m_cols, $g_cols);
		while ($row = $this->db->{$this->command}($db_result)) {
			if (!$custom_columns) {
				$key = $row[$cols[0]];
				$tmp_cols = array_slice($cols, 1);
				$data = Array();
				foreach ($tmp_cols as $c) {
					$data[$c] = $row[$c];
				}
				$result[$key] = $data;
			} else {
				$tmp = Array();
				foreach ($cols as $c) {
					$tmp[$c] = $row[$c];
				}
				$result[] = $tmp;
			}
		}
		$this->db->{$this->command_free}($db_result);

		$result = $this->_maintainDataFormat($result);

		return $result;

	}//end getAllInfo()


	/**
	* Returns the summary of members/groups in multi dimen array
	*
	* The returned array can take two different forms. If cols is not specified,
	* the associative array have the id of members/groups as a key. For example,
	* <pre>
	* Array(
	*	'id'	=> Array(
	*				'name'		=> value,
	*				'mgroup		=> value,
	*				...
	*				),
	* );
	* </pre>
	* If the list of wanted columns is passed, no key is specified in the returned array
	* <pre>
	* Array(
	*	0	=> Array(
	*			'id'	=> value,
	*			'name'	=> value,
	*			'mgroup	=> value,
	*			...
	*		  ),
	*	...
	* );
	* </pre>
	*
	* @param string	$table_name	the name of the table
	* @param array	$cols		the list of wanted columns
	* @param int	$id			optionally the id of member/group can be specified
	*
	* @return boolean
	* @access public
	*/
	function getMemberGroupInfo($table_name='members', $cols=Array(), $id=0)
	{
		// add column names
		$columns		= Array();
		$custom_columns	= FALSE;
		$member_id_field = $this->_getMemberIdField();
		$member_group_id_field = $this->_getMemberGroupIdField();

		if ($table_name == 'members') {
			$table_short_name='u';
		} else if ($table_name == 'groups') {
			$table_short_name='g';
		}

		switch($this->version) {
			case '3_0_2':
				if (empty($cols)) {
					if ($table_name == 'members') {
						$cols = 'u.'.$member_id_field.', u.name, u.'.$member_group_id_field.', u.email, u.title, u.member_login_key, u.bday_day, u.bday_month, u.bday_year, \'\' as vdir, me.field_3 as aim_name, me.field_4 as msnname, me.field_5 as website, me.field_6 as icq_number, me.field_8 as location, me.field_9 as interests, me.field_10 as yahoo, ue.avatar_type as photo_type, ue.avatar_location as photo_location, ue.avatar_size as photo_dimensions, ue.notes, ue.links, ue.bio, ue.ta_size, ue.signature, ue.avatar_location, ue.avatar_size, ue.avatar_type '; 
					} else if ($table_name == 'groups') {
						$cols = 'g.g_id, g.g_title, g.g_mem_info, g.prefix, g.suffix ';
					}//end if
				} else {
					$custom_columns = TRUE;
				}//end if
				
				$from = $this->db->obj['sql_tbl_prefix'].$table_name.' '.$table_short_name;
				$join = '';
				if ($table_name == 'members') {
					$join  = ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'profile_portal ue ON u.'.$member_id_field.'=ue.pp_member_id';
					$join .= ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'pfields_content me ON me.member_id=u.'.$member_id_field;
				}
				// build query
				$where = '';
				if ($id) {
					if ($table_name == 'members') {
						$where = ' WHERE '.$table_short_name.'.'.$member_id_field.'='.$id;
					} else if ($table_name == 'groups') {
						$where = ' WHERE '.$table_short_name.'.g_id='.$id;
					}
				}
				$sql = 'SELECT '.$cols.$from.$join.$where;
			break;
			default:
				if (empty($cols)) {
					if ($table_name == 'members') {
						$cols = Array(
									'u.'.$member_id_field,
									'u.name',
									'u.'.$member_group_id_field,
									'u.email',
									'u.title',
									'u.member_login_key',
									'u.bday_day',
									'u.bday_month',
									'u.bday_year',
									'ue.photo_type',
									'ue.photo_location',
									'ue.aim_name',
									'ue.icq_number',
									'ue.website',
									'ue.yahoo',
									'ue.interests',
									'ue.msnname',
									'ue.vdirs',
									'ue.location',
									'ue.signature',
									'ue.avatar_location',
									'ue.avatar_size',
									'ue.avatar_type',
								);
					} else if ($table_name == 'groups') {
						$cols = Array('g.g_id', 'g.g_title', 'g.g_mem_info', 'g.prefix', 'g.suffix');
					}
				} else {
					$custom_columns = TRUE;
				}

				$columns = $cols;

				$join = '';
				if ($table_name == 'members') {
					$join = ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'member_extra ue ON u.'.$member_id_field.'=ue.id';
				}
				// build query
				$where = '';
				if ($id) {
					if ($table_name == 'members') {
						$where = ' WHERE '.$table_short_name.'.'.$member_id_field.'='.$id;
					} else if ($table_name == 'groups') {
						$where = ' WHERE '.$table_short_name.'.g_id='.$id;
					}
				}
				$sql = $this->_buildQuery($columns, $table_name, $table_short_name, $where, $join);
		}//end switch

		// get the result
		$db_result = $this->db->query($sql);
		$this->_assertValidResult($this->db, 'Could not get IPB member group info');

		$result = Array();
		while ($row = $this->db->{$this->command}($db_result)) {
			if (!$custom_columns) {
				$tmp_col_name = explode('.', $cols[0]);
				$key = $row[$tmp_col_name[1]];
				$tmp_cols = array_slice($cols, 1);
				$data = Array();
				foreach ($tmp_cols as $c) {
					$tmp_col_name = explode('.', $c);
					$data[$tmp_col_name[1]] = $row[$tmp_col_name[1]];
				}
				$data['type'] = $table_name;
				$result[$table_short_name.'_'.$key] = $data;
			} else {
				$tmp = Array('type' => $table_name);
				foreach ($cols as $c) {
					if ($c == $member_id_field || $c == 'g_id') {
						$tmp[$c] = $table_short_name.'_'.$row[$c];
					} else {
						$tmp[$c] = $row[$c];
					}
				}
				$result[] = $tmp;
			}
		}
		$this->db->{$this->command_free}($db_result);

		$result = $this->_maintainDataFormat($result);

		return $result;

	}//end getMemberGroupInfo()


	/**
	* Returns all the members or a group of members according to
	* the given arguments
	*
	* @param int	$member_id	matrix ipb_user_id which maps to IPB member id
	* @param int	$group_id	matrix ipb_group_id which matps to IPB group id
	* @param array	$cols		the list of fields
	*
	* @return array
	* @access public
	*/
	function getMembers($member_id=0, $group_id=0, $cols=Array())
	{
		// add column names
		$columns		= Array();
		$custom_columns	= FALSE;
		$table_name = 'members';
		$table_short_name='m';
		$member_id_field = $this->_getMemberIdField();
		$member_group_id_field = $this->_getMemberGroupIdField();
		if (empty($cols)) {
			$cols = Array($member_id_field, 'name', $member_group_id_field, 'email', 'title', 'member_login_key');
		} else {
			$custom_columns = TRUE;
		}

		// add prefix
		foreach ($cols as $c) {
			$columns[] = $table_short_name.'.'.$c;
		}

		// build query
		$where = '';
		if ($member_id || $group_id) {
			$where = ' WHERE ';
			if ($member_id) {
				$where .= $table_short_name.'.'.$member_id_field.'='.$member_id.(($group_id) ? ' and ' : '');
			}
			if ($group_id) {
				$where .= $table_short_name.'.'.$member_group_id_field.'='.$group_id;
			}
		}

		$sql = $this->_buildQuery($columns, $table_name, $table_short_name, $where);

		// get the result
		$db_result = $this->db->query($sql);
		$this->_assertValidResult($this->db, 'Could not get IPB member(s)');

		$result = Array();
		while ($row = $this->db->{$this->command}($db_result)) {
			if (!$custom_columns) {
				$key = $row[$cols[0]];
				$tmp_cols = array_slice($cols, 1);
				$data = Array();
				foreach ($tmp_cols as $c) {
					if ($c == $member_group_id_field) {
						$data[$c] = 'g_'.$row[$c];
					} else {
						$data[$c] = $row[$c];
					}
				}
				$data['type'] = $table_name;
				$result['u_'.$key] = $data;
			} else {
				$tmp = Array('type' => $table_name);
				foreach ($cols as $c) {
					if ($c == $member_id_field) {
						$tmp[$c] = 'u_'.$row[$c];
					} else {
						$tmp[$c] = $row[$c];
					}
				}
				$result[] = $tmp;
			}
		}
		$this->db->{$this->command_free}($db_result);

		$result = $this->_maintainDataFormat($result);

		return $result;

	}//end getMembers()


	/**
	* Returns the list of all custom fields for the members
	* if there are any
	*
	* @return array
	* @access public
	*/
	function getCustomFields()
	{
		$sql = $this->_buildQuery('*', 'pfields_data', 'pfd');

		return $this->_getResult($sql, Array(), FALSE);

	}//end getCustomFields()


	/**
	* Returns the list of all custom fields for the members
	* if there are any
	*
	* @param int	$member_id	the id of the user
	*
	* @return array
	* @access public
	*/
	function getMemberCustomFieldData($member_id)
	{
		$tmp = explode(':', $member_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = $this->_buildQuery('*', 'pfields_content', 'pfc', ' WHERE pfc.member_id='.$id);

		return $this->_getResult($sql, Array(), FALSE);

	}//end getMemberCustomFieldData()


//--        GROUPS        --//


	/**
	* Returns the details of the Ipb User Group
	*
	* @param int	$group_id	the id of the ipb user group
	*
	* @return array
	* @access public
	*/
	function getGroupInfo($group_id)
	{
		$tmp = explode(':', $group_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = $this->_buildQuery('*', 'groups', 'g', ' WHERE g.g_id='.$id);
		return $this->_getResult($sql, Array(), TRUE);

	}//end getGroupInfo()


	/**
	* Returns the list of all Ipb User Groups
	*
	* @return array
	* @access public
	*/
	function getListOfGroups()
	{
		$fields = Array(
					'g.g_id',
					'g.g_title',
				  );

		$sql = $this->_buildQuery($fields, 'groups', 'g');

		return $this->_getResult($sql, Array('g_id' => 'id'), FALSE);

	}//end getListOfGroups()


//--        MEMBERS        --//


	/**
	* Returns TRUE if the user's group has the access
	* to view the IPB board
	*
	* @param int	$username	the name of the ipb user
	*
	* @return array
	* @access public
	*/
	function canViewBoard($username)
	{
		$member_group_id_field = $this->_getMemberGroupIdField();
		$sql = 'SELECT
					m.name, g.g_view_board
				FROM
					'.$this->db->obj['sql_tbl_prefix'].'members m,
					'.$this->db->obj['sql_tbl_prefix'].'groups g
				WHERE
					m.'.$member_group_id_field.'=g.g_id and
					m.name=\''.str_replace('&#039', '&#39', htmlspecialchars($username, ENT_QUOTES)).'\'
				;';
		return $this->_getResult($sql, Array(), TRUE);

	}//end canViewBoard()


	/**
	* Returns the details of Ipb User
	*
	* @param int	$member_id	the id of the ipb user
	*
	* @return array
	* @access public
	*/
	function getMemberInfo($member_id)
	{
		$tmp = explode(':', $member_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];
		switch ($this->version) {
			case '3_0_2':
				$sql = 'SELECT m.*, \'\' as vdir, me.field_3 as aim_name, me.field_4 as msnname, me.field_5 as website, me.field_6 as icq_number, me.field_8 as location, me.field_9 as interests, me.field_10 as yahoo, ue.avatar_type as photo_type, ue.avatar_location as photo_location, ue.avatar_size as photo_dimensions, ue.notes, ue.links, ue.bio, ue.ta_size, ue.signature, ue.avatar_location, ue.avatar_size, ue.avatar_type FROM '.$this->db->obj['sql_tbl_prefix'].'members m LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'profile_portal ue ON ue.pp_member_id=m.member_id LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'pfields_content me ON me.member_id=m.member_id WHERE m.member_id='.$id;
			break;
			default:
				$sql = $this->_buildQuery('*', 'members', 'm', ' WHERE m.id='.$id, ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'member_extra me ON me.id=m.id');
		}//end switch

		$data = $this->_getResult($sql, Array(), TRUE, TRUE);

		return $data;

	}//end getMemberInfo()


	/**
	* Returns the details of Ipb User by Name
	*
	* @param int	$member_name	the name of the ipb user
	*
	* @return array
	* @access public
	*/
	function getMemberInfoByName($member_name)
	{
		switch ($this->version) {
			case '3_0_2':
				$member_id_field = $this->_getMemberIdField(); 
				$sql = 'SELECT m.*, \'\' as vdir, me.field_3 as aim_name, me.field_4 as msnname, me.field_5 as website, me.field_6 as icq_number, me.field_8 as location, me.field_9 as interests, me.field_10 as yahoo, ue.avatar_type as photo_type, ue.avatar_location as photo_location, ue.avatar_size as photo_dimensions, ue.notes, ue.links, ue.bio, ue.ta_size, ue.signature, ue.avatar_location, ue.avatar_size, ue.avatar_type FROM '.$this->db->obj['sql_tbl_prefix'].'members m LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'profile_portal ue ON ue.pp_member_id=m.member_id LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'pfields_content me ON me.member_id=m.member_id WHERE m.name=\''.str_replace('&#039', '&#39', htmlspecialchars($member_name, ENT_QUOTES)).'\'';
			break;
			default:
				$sql = $this->_buildQuery('*', 'members', 'm', ' WHERE m.name=\''.str_replace('&#039', '&#39', htmlspecialchars($member_name, ENT_QUOTES)).'\'', ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'member_extra me ON me.id=m.id');
		}//end switch

		$data = $this->_getResult($sql, Array(), TRUE, TRUE);

		return $data;

	}//end getMemberInfoByName()


	/**
	* Returns all the groups which this user belongs to
	*
	* @param int	$member_id	the id of the ipb user
	*
	* @return array
	* @access public
	*/
	function getMemberParent($member_id)
	{
		$member_id_field = $this->_getMemberIdField();
		$member_group_id_field = $this->_getMemberGroupIdField();
		switch ($this->version) {
			case '3_0_2':
				$member_id_field = 'member_id';
				$member_group_id_field = 'member_group_id';
			break;
			default:
				$member_id_field = 'id';
				$member_group_id_field = 'mgroup';
		}//end switch
		$tmp = explode(':', $member_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = $this->_buildQuery(Array($member_group_id_field), 'members', 'm', ' WHERE m.'.$member_id_field.'='.$id);

		$data = $this->_getResult($sql, Array(), TRUE, TRUE);

		return $data;

	}//end getMemberParent()


	/**
	* Returns the list of all members in this Ipb User Group
	*
	* @param int	$group_id	the id of the ipb user group
	*
	* @return array
	* @access public
	*/
	function getListOfMembers($group_id=0)
	{
		$member_id_field = $this->_getMemberIdField();
		$member_group_id_field = $this->_getMemberGroupIdField();
		$fields = Array(
					'u.'.$member_id_field,
					'u.name',
					'u.'.$member_group_id_field,
				  );

		if ($group_id== 0) {
			$sql = $this->_buildQuery($fields, 'members', 'u');
		} else {
			$tmp = explode(':', $group_id);
			if (!isset($tmp[1])) return Array();
			$tmp = explode('_', $group_id);
			$id = $tmp[1];
			$sql = $this->_buildQuery($fields, 'members', 'u', ' WHERE u.'.$member_group_id_field.'='.$id);
		}

		$results = $this->_getResult($sql, Array(), FALSE);

		// Different format, so extra cleaning is involved
		foreach ($results as $index => $result) {
			$results[$index] = $this->_maintainDataFormat($result);
		}//end foreach

		return $results;

	}//end getListOfMembers()


//--        ROOT FORUMS        --//


	/**
	* Returns the details of Ipb Root Forum
	*
	* @param int	$forum_id	the id of the ipb root forum
	*
	* @return array
	* @access public
	*/
	function getRootForumInfo($forum_id)
	{
		$tmp = explode(':', $forum_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$fields = Array(
					'f.id',
					'f.name',
					'f.description',
				  );
		$sql = $this->_buildQuery($fields, 'forums', 'f', ' WHERE f.id='.$id);

		return $this->_getResult($sql, Array(), TRUE);

	}//end getRootForumInfo()


	/**
	* Returns the list of all Ipb Root Forums
	*
	* @return array
	* @access public
	*/
	function getListOfRootForums()
	{
		$fields = Array(
					'f.id',
					'f.name',
				  );
		$sql = $this->_buildQuery($fields, 'forums', 'f', ' WHERE f.parent_id=-1');

		return $this->_getResult($sql, Array(), FALSE);

	}//end getListOfRootForums()


//--        FORUMS        --//


	/**
	* Returns the details of Ipb Forum
	*
	* @param int	$forum_id	the id of the ipb forum
	*
	* @return array
	* @access public
	*/
	function getForumInfo($forum_id)
	{
		$tmp = explode(':', $forum_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$fields = Array(
					'f.id',
					'f.name',
					'f.description',
					'f.topics',
					'f.parent_id',
					'f.last_post',
					'f.last_poster_name',
				  );
		$sql = $this->_buildQuery($fields, 'forums', 'f', ' WHERE f.id='.$id);

		return $this->_getResult($sql, Array(), TRUE);

	}//end getForumInfo()


	/**
	* Returns the root forum which this forum belongs to
	*
	* @param int	$forum_id	the id of the ipb forum
	*
	* @return array
	* @access public
	*/
	function getForumParent($forum_id)
	{
		$tmp = explode(':', $forum_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = $this->_buildQuery(Array('parent_id'), 'forums', 'f', ' WHERE f.id='.$id);

		return $this->_getResult($sql, Array(), TRUE);

	}//end getForumParent()


	/**
	* Returns all the forum under this root forum
	*
	* @param int	$root_forum_id	the id of the ipb forum
	*
	* @return array
	* @access public
	*/
	function getListOfForums($root_forum_id)
	{
		$tmp = explode('_', $root_forum_id);
		$id = $tmp[1];

		$fields = Array(
					'f.id',
					'f.name',
					'f.description',
				  );
		$sql = $this->_buildQuery($fields, 'forums', 'f', ' WHERE f.parent_id='.$id);

		return $this->_getResult($sql, Array(), FALSE);

	}//end getListOfForums()


//--        TOPICS        --//


	/**
	* Returns the details of this Ipb Topic
	*
	* @param int	$topic_id	the id of the ipb topic
	*
	* @return array
	* @access public
	*/
	function getTopicInfo($topic_id)
	{
		$tmp = explode(':', $topic_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$fields = Array(
					't.tid',
					't.title',
					't.forum_id',
					't.description',
					't.posts',
					't.starter_name',
					't.start_date',
					't.last_poster_name',
					't.last_post',
					't.poll_state',
				  );
		$sql = $this->_buildQuery($fields, 'topics', 't', ' WHERE t.tid='.$id);

		return $this->_getResult($sql, Array('title' => 'name'), TRUE);

	}//end getTopicInfo()


	/**
	* Returns the forum which this topic belongs to
	*
	* @param int	$topic_id	the id of the ipb topic
	*
	* @return array
	* @access public
	*/
	function getTopicParent($topic_id)
	{
		$tmp = explode(':', $topic_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = 'SELECT
					t.forum_id,
					f.parent_id
				FROM
					'.$this->db->obj['sql_tbl_prefix'].'topics t,
					'.$this->db->obj['sql_tbl_prefix'].'forums f
				WHERE
					t.forum_id=f.id
					and t.tid='.$id.';
				';

		return $this->_getResult($sql, Array(), TRUE);

	}//end getTopicParent()


	/**
	* Returns the list of all the topic under this forum
	*
	* @param int	$forum_id	the id of the ipb forum
	*
	* @return array
	* @access public
	*/
	function getListOfTopics($forum_id)
	{
		$tmp = explode('_', $forum_id);
		$id = $tmp[1];

		$fields = Array(
					't.tid',
					't.title',
					't.description',
				  );
		$where = ' WHERE t.forum_id='.$id.' AND o.pid IS NULL';
		$join =  ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'polls o ON t.tid=o.tid';
		$sql = $this->_buildQuery($fields, 'topics', 't', $where, $join);

		return $this->_getResult($sql, Array('tid' => 'id'));

	}//end getListOfTopics()


//--        POSTS        --//


	/**
	* Returns the details of this Ipb Post
	*
	* @param int	$post_id	the id of the ipb post
	*
	* @return array
	* @access public
	*/
	function getPostInfo($post_id)
	{
		$tmp = explode(':', $post_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = 'select
					p.pid,
					p.author_name,
					p.post_date,
					p.post_title,
					p.post_parent,
					p.post,
					t.tid,
					t.title as topic_name,
					f.name as forum_name
				from
					'.$this->db->obj['sql_tbl_prefix'].'posts p,
					'.$this->db->obj['sql_tbl_prefix'].'topics t,
					'.$this->db->obj['sql_tbl_prefix'].'forums f
				where
					p.pid='.$id.'
					and p.topic_id = t.tid
					and t.forum_id = f.id
				order by
					post_date desc;';

		return $this->_getResult($sql, Array(), TRUE);

	}//end getPostInfo()


	/**
	* Returns the topic which this topic belongs to
	*
	* @param int	$post_id	the id of the ipb post
	*
	* @return array
	* @access public
	*/
	function getPostParent($post_id)
	{
		$tmp = explode(':', $post_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$sql = 'SELECT
					t.tid AS topic_id,
					t.forum_id,
					f.parent_id
				FROM
					'.$this->db->obj['sql_tbl_prefix'].'posts p,
					'.$this->db->obj['sql_tbl_prefix'].'topics t,
					'.$this->db->obj['sql_tbl_prefix'].'forums f
				WHERE
					t.forum_id=f.id
					and p.topic_id=t.tid
					and p.pid='.$id.';
				';

		return $this->_getResult($sql, Array(), TRUE);

	}//end getPostParent()


	/**
	* Returns the list of all posts under this topic
	*
	* @param int	$topic_id	the id of the ipb topic
	* @param int	$forum_id	the id of the ipb forum
	* @param int	$limit		optional limit clause
	*
	* @return array
	* @access public
	*/
	function getListOfPosts($topic_id, $forum_id=NULL, $limit=0)
	{
		if (is_null($forum_id)) {
			$tmp = explode('_', $topic_id);
			$id = $tmp[1];
			$fields = Array(
						'p.pid',
					  );
			if ($limit != 0) {
				$sql = $this->_buildQuery($fields, 'posts', 'p', ' WHERE p.topic_id='.$id, ' LIMIT '.$limit);
			} else {
				$sql = $this->_buildQuery($fields, 'posts', 'p', ' WHERE p.topic_id='.$id);
			}
		} else {
			$sql = 'SELECT
						p.pid, t.tid, p.author_id, p.post_date
					FROM
						'.$this->db->obj['sql_tbl_prefix'].'posts p,
						'.$this->db->obj['sql_tbl_prefix'].'topics t
					WHERE
						p.topic_id=t.tid AND
						t.forum_id='.$forum_id.'
					ORDER BY
						p.post_date DESC
			';
			if ($limit != 0) $sql .= ' LIMIT '.$limit;
			$sql .= ';';
		}

		return $this->_getResult($sql, Array('pid' => 'id'));

	}//end getListOfPosts()


//--        POLLS        --//


	/**
	* Returns the details of this poll
	*
	* @param int	$poll_id	the id of the ipb poll
	*
	* @return array
	* @access public
	*/
	function getPollInfo($poll_id)
	{
		$tmp = explode(':', $poll_id);
		if (!isset($tmp[1])) return Array();
		$tmp = explode('_', $tmp[1]);
		$id = $tmp[1];

		$fields = Array(
					'o.pid',
					'o.tid',
					'o.start_date',
					'o.choices',
					'o.starter_id',
					'o.votes',
					'o.forum_id',
					'o.poll_question',
					't.poll_state as poll_state',
				  );
		$join =  ' LEFT JOIN '.$this->db->obj['sql_tbl_prefix'].'topics t ON t.tid=o.tid';
		$sql = $this->_buildQuery($fields, 'polls', 'o', ' WHERE o.pid='.$id, $join);

		return $this->_getResult($sql, Array(), TRUE);

	}//end getPollInfo()


	/**
	* Returns the list of all polls
	*
	* @return array
	* @access public
	*/
	function getListOfPolls()
	{
		$sql = $this->_buildQuery('*', 'polls', 'o');

		return $this->_getResult($sql, Array('pid' => 'id'));

	}//end getListOfPolls()


//--        QUERIES        --//


	/**
	* Return sql query string
	*
	* @param array	$columns			the list of columns
	* @param string	$table_name			the name of the table
	* @param string	$table_short_name	the short name of the table
	* @param string	$where				WHERE clauss
	* @param string	$join				JOIN clause
	* @param string	$order_by			ORDER BY clause
	* @param string	$limit				LIMIT clause
	*
	* @return boolean
	* @access private
	*/
	function _buildQuery($columns, $table_name, $table_short_name, $where='', $join='', $order_by='', $limit='')
	{
		if ($columns == '*') {
			$sql = 'SELECT * ';
		} else {
			$sql = 'SELECT '.implode(', ', $columns);
		}

		$sql .= ' FROM '.$this->db->obj['sql_tbl_prefix'].$table_name.' '.$table_short_name;

		// JOIN clause
		if (!empty($join)) $sql .= $join;

		// WHERE clause
		if (!empty($where)) $sql .= $where;

		// ORDER BY clause
		if (!empty($order_by)) $sql .= $order_by;

		// LIMIT clause
		if (!empty($limit)) $sql .= $limit;

		$sql .= ';';

		return $sql;

	}//end _buildQuery()


	/**
	* Return the db execution result
	*
	* @param string		$sql			the sql query to execute
	* @param array		$key_change		The caller can specify the name of the key
	*									by providing the pair of the old and new key
	* @param boolean	$one_element	If the result only contains one element, it's TRUE
	* @param boolean	$backward		If TRUE, make sure the data in the array is backward compatible
	*
	* @return array
	* @access private
	*/
	function _getResult($sql, $key_change=Array(), $one_element=FALSE, $backward=FALSE)
	{
		$db_result = $this->db->query($sql);
		$this->_assertValidResult($this->db, 'Could not get result from IPB');

		$result = Array();
		while ($row = $this->db->{$this->command}($db_result)) {
			if (!empty($key_change)) {
				foreach ($key_change as $old_key => $new_key) {
					$row[$new_key] = $row[$old_key];
				}
			}
			$result[] = $row;
		}
		$this->db->{$this->command_free}($db_result);
		if (empty($result)) return Array();
		if ($one_element) $result = array_pop($result);

		if ($backward) {
			$result = $this->_maintainDataFormat($result);
		}//end if

		return $this->_htmlDecode($result);

	}//end _getResult()


//--        HTML DECODE        --//


	/**
	* Convert html encoded characters
	*
	* This function is useful since IPB stores html encoded characters
	* in the database. All the results from database go through this
	* function to make sure they do not contain any html encoded chars
	*
	* @param string	$result	the string to convert
	*
	* @return string
	* @access private
	*/
	function _htmlDecode($result)
	{

		// list from post_parser.php
		$html_code_table = Array(
							'&#39;'					=> '\'',
							'&#039;'				=> '\'',
							'&#33;'					=> '!',
							'&#036;'				=> '$',
							'&#124;'				=> '|',
							'&amp;'					=> '&',
							'&gt;'					=> '>',
							'&lt;'					=> '<',
							'&quot;'				=> '"',
							'&#153;'				=> '(tm)',
							'&#58;'					=> ':',
							'&#91;'					=> '[',
							'&#93;'					=> ']',
							'&#41;'					=> ')',
							'&#40;'					=> '(',
							'&#60;'					=> '<',
							'&#62;'					=> '>',
							'&#043;'				=> '+',
							'&#045;'				=> '-',
							'j&#097;v&#097;script'	=> 'javascript',
							'&#097;lert'			=> 'alert',
							'&#097;lert'			=> 'about:',
							'&#111;nmouseover'		=> 'onmouseover',
							'&#111;nclick'			=> 'onclick',
							'&#111;nload'			=> 'onload',
							'&#111;nsubmit'			=> 'onsubmit',
						   );

		foreach ($result as $key => $value) {
			if (is_string($value)) {
				foreach ($html_code_table as $html_code => $char) {
					$new_result[$key] = str_replace($html_code, $char, $value);
					$value = $new_result[$key];
				}
			} else {
				$new_result[$key] = $value;
			}
		}

		return $new_result;

	}//end _htmlDecode()


	/**
	* Asserts a valid DB result
	*
	* This has to be done in a different way because the DB in question is
	* an IPB DB object, not a Matrix one.
	*
	* @param DB_Driver	$db			The DB object
	* @param string		$message	Message to display if failed
	*
	* @return void
	*/
	protected function _assertValidResult($db, $message)
	{
		if (!empty($db->error)) {
			trigger_error($message.': '.$db->error, E_USER_ERROR);
		}

	}//end _assertValidResult()


	/**
	 * Return the member_id (used to maintain backwards compatability)
	 *
	 * @return string
	 * @access private
	 */
	protected function _getMemberIdField()
	{
		switch ($this->version) {
			case '3_0_2':
				return 'member_id';
			default:
				return 'id';
		}//end switch
	
	}//end _getMemberIdField()


	/**
	 * Return the member_group_id (used to maintain backwards compatability)
	 *
	 * @return string
	 * @access private
	 */
	protected function _getMemberGroupIdField()
	{
		switch ($this->version) {
			case '3_0_2':
				return 'member_group_id';
			default:
				return 'mgroup';
		}//end switch
	
	}//end _getMemberIdField()


	/**
	 * Maintain data format (used to maintain backwards compatability)
	 *
	 * @param array	$data	The array to maintain
	 *
	 * @return string
	 * @access private
	 */
	protected function _maintainDataFormat($data=Array())
	{
		switch ($this->version) {
			case '3_0_2':
				if (isset($data['member_id'])) {
					$data['id'] = $data['member_id'];
				}//end if
				if (isset($data['member_group_id'])) {
					$data['mgroup'] = $data['member_group_id'];
				}//end if
			break;
		}//end switch

		return $data;
	
	}//end _maintainDataFormat()


}//end class

?>
