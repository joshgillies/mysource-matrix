<?php

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Config
*
* Purpose
*
*    Looks after the creation and editing of the system config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Config extends Resolve_Object 
{

	var $config_file;

	var $config_vars = Array(
		'SQ_CONF_DB_DSN'					=> Array('editable' => 0, 'default' => ''),
		'SQ_CONF_PEAR_PATH'					=> Array('editable' => 0, 'default' => ''),
		'SQ_CONF_BACKEND_SUFFIX'			=> Array('editable' => 0, 'default' => '_edit'),
/*
* Fixed Size of the number of digits that each link trees entry can be, run 
*   pow(SQ_CONF_ASSET_TREE_BASE, SQ_CONF_ASSET_TREE_SIZE) 
* to get max number of possible link entries per node
*/
		'SQ_CONF_ASSET_TREE_BASE'			=> Array('editable' => 0, 'default' => 64),
		'SQ_CONF_ASSET_TREE_SIZE'			=> Array('editable' => 0, 'default' =>  4),

		'SQ_CONF_SYSTEM_NAME'				=> Array('editable' => 1, 'default' => 'Resolve System'),
		'SQ_CONF_SYSTEM_OWNER'				=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_SYSTEM_ADMIN_EMAIL'		=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_SYSTEM_ROOT_URLS'			=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_SUPER_USERS'				=> Array('editable' => 1, 'default' => ''),
		'SQ_CONF_MAX_IDLE_TIME'				=> Array('editable' => 1, 'default' => '6000'),
		'SQ_CONF_MAX_LOGIN_ATTEMPTS'		=> Array('editable' => 1, 'default' => '3'),
		'SQ_CONF_DEFAULT_FRONTEND_LANGUAGE'	=> Array('editable' => 1, 'default' => 'en'),
		'SQ_CONF_DEFAULT_CHARACTER_SET'		=> Array('editable' => 1, 'default' => 'iso-8859-1'),
		'SQ_CONF_ERRORS_HIDE_FRONTEND'		=> Array('editable' => 1, 'default' => '0'),
		'SQ_CONF_ERRORS_LOG'				=> Array('editable' => 1, 'default' => '0'),
		'SQ_CONF_REFRESH_INTERVAL'			=> Array('editable' => 1, 'default' => '120')
	);




/*
	var $system_name = 'MySource System';
	var $system_owner = 'System Owner';
	var $webmaster_email;
	var $backend_message_box_focus = 0;
	var $backend_log_messages = '';
	var $backend_brand_image = '';
	var $jupload_path;
	

	var $superusers = array('all' => array());

	var $max_idle_time	= 6000;
	var $max_login_attempts = 3;
	var $log_visitors = 0;
	var $log_remote_hosts = 0;

	var $web_db_details = array('name'=>'','host'=>'','login'=>'','password'=>'', 'querylog'=>'');

	var $virtual_paths  = 0;
	var $strict_url_matching = 0;
	var $backend_suffix = 'edit';
	var $default_language;
	var $default_charset;
	var $moderate_caching;
	var $illegal_filetypes = 'php php3 php4 php5 php6 inc act';
	var $site_backend_dhtml_navigation = 0;
	var $default_table_cell_type = 'richtext';

	var $user_authentication = 'default';

	var $user_db_details	= array('name'=>'','host'=>'','login'=>'','password'=>'','querylog'=>'');
	
	
var $user_ldap_details  = array('host' => '', 'password' => '', 'rootdn' => '');
*/

	/**
	* Constructor
	*
	*/
	function Config() 
	{
		$this->Resolve_Object();
		$this->config_file = SQ_DATA_PATH.'/conf/main.inc';
	}

	/**
	* Rewrites the conf file with current variables
	*
	*/
	function save($vars, $backup_existing=false) {

		$str = "<"."?php\n";

		foreach($this->config_vars as $var_name => $data) {

			$value = $data['default'];
			if ($data['editable'] && isset($vars[$var_name])) {
				$value = $vars[$var_name];
			} elseif (defined($var_name)) {
				$value = constant($var_name);
			}

			$str .= "define('$var_name', ".var_export($value, true).");\n";

			if ($var_name == 'SQ_CONF_PEAR_PATH' && !empty($value)) {
				$str .= "\$sep = (substr(PHP_OS, 0, 3) == 'WIN') ? ';' : ':';\n";
				$str .= "\$inc_dir = ini_get('include_path');\n";
				// basically, if the first entry is '.' (ie the current directory) leave that, 
				// and insert the pear path just after that, otherwise just right at the front of the 
				// include path
				$str .= "\$inc_dir = (substr(\$inc_dir, 0, 2) == '.'.\$sep) ? '.'.\$sep.SQ_CONF_PEAR_PATH.\$sep.substr(\$inc_dir, 2): SQ_CONF_PEAR_PATH.\$sep.\$inc_dir;\n";
				$str .= "ini_set('include_path', \$inc_dir);\n\n";
			}

		}//end foreach

		$str .= "?>\n";

		if ($backup_existing && file_exists($this->config_file)) {
		
			$i = 0;
			do {
				$i++;
				$old_version = $this->config_file.'.'.$i;
			} while(file_exists($old_version));

			if (!copy($this->config_file, $old_version)) {
				return false;
			}

		}
		
		print_r($str);
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		return string_to_file($str, $this->config_file);
	}

	/**
	* Lets print the conf backend
	*
	* @param object Backend	$backend	Reference to the backend object
	*/
	function paintBackend(&$backend) {

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$o = &$backend->out;


/*
		#$this->save(get_defined_constants());
		 ###################################################
		# SECUIRTY - Only let in those who have permission
		if (!$SESSION->logged_in()) {
			$SESSION->login_screen($backend->title,"You must be logged in and have permission to edit <i>$this->system_name</i> in order to proceed.");
		}

		if (!superuser('web') && !superuser('user')) {
			$SESSION->login_screen($backend->title,"You do not have permission to view or edit the system config of <i>$this->system_name</i>.");
		}

		$backend->set_active_menu_item('config');

		# Only privileged users can mess around here
		if (superuser('web') || superuser('user')) {
			global $action;
		
			#############################
			# get users database object 
			#############################
			#
			$users_system = &get_users_system();
			$db = &$users_system->db;
			
			##################
			# process $action 
			##################
			#
			switch($action) {

				case 'Commit':

					global  $system_name,$system_owner,$webmaster_email,$jupload_path,$web_db_details,
							$user_db_details,$max_login_attempts,$max_idle_time,
							$delete_superuser_ids, $delete_webmaster_ids, $delete_usermaster_ids,
							$superuser_search, $webmaster_search, $usermaster_search, 
							$log_visitors, $log_remote_hosts, $virtual_paths, $backend_suffix, $strict_url_matching,
							$default_language, $default_charset, $moderate_caching, $illegal_filetypes, $check_db_mbstrings, 
							$site_backend_dhtml_navigation, $backend_message_box_focus, $backend_log_messages, 
							$user_db_details, $user_authentication, $default_table_cell_type, $batch_delete, $batch_dupe;

					if(user_root()) {

						####################
						# delete superusers	
						####################
						#
						if (count($delete_superuser_ids)) {
							$sql = "SELECT login
									  FROM user
									 WHERE userid IN ('".implode("','", $delete_superuser_ids)."')";
							
							$superusers = $db->single_column($sql);
							foreach($superusers as $login) {
								$backend->add_message($this->delete_superuser($login));
							}
						}

						#################
						# add superusers	
						#################
						#
						$user_searches = split("[ \t\r\n\,]+",gpc_stripslashes($superuser_search));
						foreach($user_searches as $login) {
							# security & empty var check
							if (trim($login) != '') {
								# make sure user exists before adding
								if ($user_row = $users_system->find_user($login)) {
									$backend->add_message($this->add_superuser($user_row['login']));
								}
								else {
									$backend->add_message("Could not add superuser: $login. User does not exist");
								}
							}
						}# end foreach

						
						###############################
						# update web database details 
						###############################
						#
						$web_db_details = gpc_stripslashes($web_db_details);
						$backend->add_message($this->set_web_db_details($web_db_details['name'],
																		$web_db_details['host'],
																		$web_db_details['login'],
																		$web_db_details['password'],
																		$web_db_details['querylog']));
						
						###############################
						# update user database details
						###############################
						#
						$user_db_details     = gpc_stripslashes($user_db_details);
						$user_authentication = gpc_stripslashes($user_authentication);

						$backend->add_message($this->set_user_db_details(
								$user_db_details['name'],
								$user_db_details['host'],
								$user_db_details['login'],
								$user_db_details['password'],
								$user_db_details['querylog']
						));

						
						$backend->add_message($this->set_user_authentication($user_authentication));

					}#end if user_root



					if(superuser()) {

						#####################
						# delete web masters
						#####################
						#
						if (count($delete_webmaster_ids)) {
							$sql = "SELECT login
									  FROM user
									 WHERE userid IN ('".implode("','", $delete_webmaster_ids)."')";
							
							$webmasters = $db->single_column($sql);
							foreach($webmasters as $login) {
								$backend->add_message($this->delete_web_master($login));
							}
						}

						######################
						# delete user masters
						######################
						#
						if (count($delete_usermaster_ids)) {
							$sql = "SELECT login
									  FROM user
									 WHERE userid IN ('".implode("','", $delete_usermaster_ids)."')";
							
							$usermasters = $db->single_column($sql);
							foreach($usermasters as $login) {
							  if (superuser()) {
								$backend->add_message($this->delete_user_master($login));
							  }
							}
						}

						##################
						# add web masters	
						##################
						#
						$user_searches = split("[ \t\r\n\,]+", gpc_stripslashes($webmaster_search));
						foreach($user_searches as $login) {
							# security and empty var check
							if (trim($login) != '') {
								# check user exists
								if ($user_row = $users_system->find_user($login)) {
									$backend->add_message($this->add_web_master($user_row['login']));
								}
								else {
									$backend->add_message("Could not add web master: $login. User does not exist");
								}
							}
						}# end foreach

						###################
						# add user masters	
						###################
						#
						$user_searches = split("[ \t\r\n\,]+", gpc_stripslashes($usermaster_search));
						foreach($user_searches as $login) {
							# security and empty string check
							if (trim($login) != '') {
								# user exists in first place?
								if ($user_row = $users_system->find_user($login)) {
									$backend->add_message($this->add_user_master($user_row['login']));
								}
								else {
									$backend->add_message("Could not add user master: $login. User does not exist");
								}
							}
						}# end foreach


						# Update other config vars
						$backend->add_message($this->set_system_name(gpc_stripslashes($system_name)));
						$backend->add_message($this->set_system_owner(gpc_stripslashes($system_owner)));
						$backend->add_message($this->set_webmaster_email(gpc_stripslashes($webmaster_email)));
						$backend->add_message($this->set_backend_message_box_focus(gpc_stripslashes($backend_message_box_focus)));

						$backend->add_message($this->set_backend_log_messages(gpc_stripslashes($backend_log_messages)));

						$upload_result = process_image_upload('backend_brand_image', get_data_path(true, 'backend_brand_image'), false);
						$backend->add_message($upload_result['message']);
						$image = get_image(get_data_path(true, 'backend_brand_image'));
						if ($image) {
							$image_size = getImageSize($image);
							$image = '<img src="'.data_href(basename($image)).'" '.$image_size[3].' border="0">';
						}
						$backend->add_message($this->set_backend_brand_image($image));

						$backend->add_message($this->set_jupload_path($jupload_path));

					}#end if superuser

					if (superuser()) {
						$backend->add_message($this->set_max_login_attempts(gpc_stripslashes($max_login_attempts)));
						$backend->add_message($this->set_max_idle_time(gpc_stripslashes($max_idle_time)));
						$backend->add_message($this->set_log_visitors(gpc_stripslashes($log_visitors)));
						$backend->add_message($this->set_log_remote_hosts(gpc_stripslashes($log_remote_hosts)));
						$backend->add_message($this->set_batch_delete(gpc_stripslashes($batch_delete)));
						$backend->add_message($this->set_batch_dupe(gpc_stripslashes($batch_dupe)));
					}
					
					if(superuser('web')) {
						
						$backend->add_message($this->set_default_table_cell_type(gpc_stripslashes($default_table_cell_type)));
						$backend->add_message($this->set_default_language(gpc_stripslashes(implode(',',$default_language))));
						$backend->add_message($this->set_default_charset(gpc_stripslashes($default_charset)));
						$backend->add_message($this->set_moderate_caching(gpc_stripslashes($moderate_caching)));

						$backend->add_message($this->set_illegal_filetypes(gpc_stripslashes($illegal_filetypes)));
						$backend->add_message($this->set_check_db_mbstrings(gpc_stripslashes($check_db_mbstrings)));
						$backend->add_message($this->set_site_backend_dhtml_navigation(gpc_stripslashes($site_backend_dhtml_navigation)));
					}

					if(superuser()) {
						#if($this->virtual_paths) {
						$backend->add_message($this->set_backend_suffix(gpc_stripslashes($backend_suffix)));
						#} 
						$backend->add_message($this->set_virtual_paths(gpc_stripslashes($virtual_paths)));
						$backend->add_message($this->set_strict_url_matching(gpc_stripslashes($strict_url_matching)));
					}
					break;

			}
			if($action) {
				$this->clear_cache($this->id);
				$this->save_to_cache($this->id);
				$backend->add_message($this->conf_updated());
			}
		}

*/

		$o->setHeading('System Configuration', 'create');
		$o->setSubHeading(SQ_CONF_SYSTEM_NAME);
		$o->addHiddenField('action');
	
/*
		 ########################################
		# this stuff comes from the config file
		$superusers  = &$this->superusers['all'];
		$webmasters  = &$this->superusers['web'];
		$usermasters = &$this->superusers['user'];
*/

		$o->openSection('System Settings');
		$o->openField('System Name');
		if(true || superuser()) {
			text_box('config[SQ_CONF_SYSTEM_NAME]', SQ_CONF_SYSTEM_NAME, 30);
		} else {
			echo SQ_CONF_SYSTEM_NAME;
		}

		$o->openField('System Owner');
		if(true || superuser()) {
			text_box('config[SQ_CONF_SYSTEM_OWNER]', SQ_CONF_SYSTEM_OWNER, 30);
		} else {
			echo SQ_CONF_SYSTEM_OWNER;
		}

		$o->openField('Sys Admin Email');
		if(true || superuser()) {
			text_box('config[SQ_CONF_SYSTEM_ADMIN_EMAIL]', SQ_CONF_SYSTEM_ADMIN_EMAIL, 30);
		} else {
			echo SQ_CONF_SYSTEM_ADMIN_EMAIL;
		}

		$o->openField('System Root URLs');
		if(true || superuser()) {
			text_area('config[SQ_CONF_SYSTEM_ROOT_URLS]', SQ_CONF_SYSTEM_ROOT_URLS, 40, 4);
		} else {
			echo nl2br(SQ_CONF_SYSTEM_ROOT_URLS);
		}


/*
		$o->open_field('Backend Message Box Focus');
		if (superuser()) {
			combo_box('config[SQ_CONF_SYSTEM_NAME]',array('1'=>'On','0'=>'Off'),);
		} else {
			echo (($this->backend_message_box_focus)?'On':'Off');
		}
		echo('<span class=smallprint><br/>This determines whether the message box in the backend pops up over the current window or not</span>');

		$backend->open_field('Backend Log Messages','one_liner');
		if (trim(strtolower(getenv('MySource_BackendLogMessages'))) != 'on') {
			echo '<span class=smallprint>This feature is not available with your current configuration.</span>';
		} else {
			if (superuser()) {
				echo text_box('backend_log_messages',$this->backend_log_messages,30,'','class=data');
			} else {
				echo $this->backend_log_messages;
			}
			echo '<span class=smallprint><br/>/path/to/log/file or leave blank for no logging.</span>';
		}

		$backend->open_field('Backend Brand Image','one_liner');
		if (superuser()) {
			echo image_upload('backend_brand_image', get_data_path(true, 'backend_brand_image'), data_href(), 20000, 200, 100, true);
		} else {
			echo $this->backend_brand_image;
		}

		$backend->open_field('JUpload Path','one_liner');
		echo text_box('jupload_path',$this->jupload_path,40,100);
		echo '<span class=smallprint><br/>If you have configured tomcat or another Java Servlet handler on your server and have enabled the squizlib/jupload directory, enter the appropriate path here (from the browsers POV).<br/>E.g. <i>http://beta.squiz.net/agi_test_jupload/</i><br/><b>Leave this field blank if you cannot or do not wish to use the Java uploader.</b></span>';
		 
		 ##############
		# super-users
		if (user_root()) {

			if (count($superusers)) {

				$backend->open_field('Current Super Users ('.count($superusers).'):');

				$sql = "SELECT userid, login, CONCAT(firstname, ' ', surname) as name, email
						FROM user
						WHERE login IN ('".implode("','", $superusers)."')
						ORDER BY surname, firstname";
				$superusers  = $db->associative_array($sql);

				$users_system->display_users_table(array_keys($superusers), ((user_root())?'delete_superuser_ids':''), 'Delete?','super_page_counter');
			}#end if count(superusers)
		
			$backend->open_field('Add Super Users:<br/><span class=smallprint>(logins/emails)</span>');
			echo text_area(superuser_search,'',30,3,3000,'class=backend_data');

		}#end if user_root
*/

		// login and session timeout configuration
		$o->openSection('Login/Session Settings');
		$o->openField('Max Login Attempts', '', 'Zero (0) is unlimited attempts');
		if(true || superuser()) {
			text_box('config[SQ_CONF_MAX_LOGIN_ATTEMPTS]', SQ_CONF_MAX_LOGIN_ATTEMPTS, 5, 5);
		} else {
			echo SQ_CONF_MAX_LOGIN_ATTEMPTS;
		}

		$o->openField('Max Idle Time', '', 'In seconds - Min: 60 or zero (0) for unlimited idle time');
		if(true || superuser()) {
			echo text_box('config[SQ_CONF_MAX_IDLE_TIME]', SQ_CONF_MAX_IDLE_TIME, 10, 10);
		} else {
			echo SQ_CONF_MAX_IDLE_TIME;
		}

/*
		$o->open_field('Log Visitors','one_liner');
		if (trim(strtolower(getenv('MySource_LogVisitors'))) != 'on') {
			echo '<span class=smallprint>This feature is not available with your current configuration.</span>';
		} else {
			if(superuser()) {
				echo combo_box('log_visitors', array('1'=>'On','0'=>'Off'), $this->log_visitors);
			} else {
				echo (($this->log_visitors) ? 'Yes' : 'No');
			}
			echo('<span class=smallprint><br/>Turning on this setting, enables logging of sessions, page hits and file hits.<br/><i>NOTE: On high traffic sites enabling this option can cause the database server to crash due to the huge amounts of data being stored</i></span>');
		}

		$backend->open_field('Log Remote Hosts','one_liner');
		if(superuser()) {
			echo combo_box('log_remote_hosts',array('1'=>'On','0'=>'Off'),$this->log_remote_hosts);
		} else {
			echo (($this->log_remote_hosts)?'Yes':'No');
		}
		echo('<span class=smallprint><br/>If pages sometimes take a very long time to load at the beginning of a session, turn this option off. If on, the <i>hostname</i> of the user is logged, which can be used to figure out their ISP and country of origin.<br/>Only effective if Log Visitors in enabled</span>');
*/

/*
		 ############
		# WEB SYSTEM
		$o->openSection('Web System Settings');

		$backend->open_field('Virtual Paths','one_liner');
		if (superuser()) {
			echo combo_box('virtual_paths',array('1'=>'On','0'=>'Off'),$this->virtual_paths);
		} else {
			echo (($this->virtual_paths)?'On':'Off');
		}
		$backend->open_field('Backend Suffix','one_liner');
		#if(superuser() && $this->virtual_paths) {
		echo '<span class=backend_data>/</span>';
		echo text_box('backend_suffix',$this->backend_suffix,15,255,'class=backend_data');
		echo ('<span class=smallprint><br/>This must correspond with your web server configuration.</span>');
		#} else {
		#	echo "<span class=backend_data>/$this->backend_suffix</span>";
		#}

		$backend->open_field('Batch Page Delete','one_liner');
		if(superuser()) {
			echo text_box('batch_delete',(($this->batch_delete)?$this->batch_delete:'10'), 10, 10,'class=data');
		} else {
			echo (($this->batch_delete)?$this->batch_delete:'10');
		}
		echo ('<span class=smallprint><br/>The number of pages to delete at a time when deleting a page or site. Set this to a low number if your deleting is timing out.</span>');

		$backend->open_field('Batch Page Dupe','one_liner');
		if(superuser()) {
			echo text_box('batch_dupe',(($this->batch_dupe)?$this->batch_dupe:'10'), 10, 10,'class=data');
		} else {
			echo (($this->batch_dupe)?$this->batch_dupe:'10');
		}
		echo ('<span class=smallprint><br/>The number of pages to dupe at a time when duping a page or site. Set this to a low number if your duping is timing out.</span>');

		$backend->open_field('Strict URL Matching','one_liner');	
		if (superuser()) {
			echo combo_box('strict_url_matching',array('1'=>'On','0'=>'Off'),$this->strict_url_matching);
		} else {
			echo (($this->strict_url_matching)?'On':'Off');
		}
		echo ('<span class=smallprint><br/>This decides whether the system will try to match the closest URL for or display the \'Page Not Found\'.</span>');

		$backend->open_field('Moderate Caching','one_liner');	
		if (superuser()) {
			echo combo_box('moderate_caching',array('1'=>'On','0'=>'Off'),$this->moderate_caching);
		} else {
			echo (($this->moderate_caching)?'On':'Off');
		}
		echo ('<span class=smallprint><br/>MySource can attempt to control web caching in browsers and proxies using HTTP headers. You may switch this off to use the PHP defaults, however, but these tend to discourage caching which can lead to increased traffic and download time.</span>');

		$backend->open_field('Illegal Filetypes','one_liner');	
		if (superuser()) {
			echo text_area('illegal_filetypes',$this->illegal_filetypes,40,3);
		} else {
			echo $this->illegal_filetypes;
		}
		echo ('<span class=smallprint><br/>List the file extensions that you do not want MySource users to be able to upload, and use to potentially exploit your server. (all lower-case - uppercase files will also be rejected)</span>');

		# DEFAULT TABLE CELL TYPE
		if (superuser('web')) {
			$backend->open_field('Default Table Cell Type','one_liner');
			echo combo_box('default_table_cell_type',array('richtext'=>'Rich Text','wysiwyg'=>'WYSIWYG','rawhtml'=>'Raw HTML'),$this->default_table_cell_type);
		}

		# LANGUAGE
		$backend->open_field('Frontend Language','one_liner');
		$languages_config = &get_config('languages');
		if(superuser()) {
			if($this->default_language) {
				echo $languages_config->name_list($this->default_language).'<br/>';
			}
			echo multiple_combo_box('default_language',$languages_config->languages,explode(',',$this->default_language),'class=backend_data', '', 4);
		} else {
			echo '<span class=backend_data>'.$languages_config->name_list($this->default_language).'</span>';
		}		
		echo('<span class=smallprint><br/>This is the default language used on the frontend. It may be overridden by individual sites and pages.</span>');

		# CHARACTER SETS
		$backend->open_field('Character Set','one_liner');
		if(superuser()) {
			$charsets_config = &get_config('charsets');
			echo combo_box('default_charset',array(''=>'-- Browser Default --')+$charsets_config->charsets,$this->default_charset, 'class=backend_data');
		} else {
			$languages_config = &get_config('charsets');
			echo "<span class=backend_data>{$charsets_config->charsets[$this->default_charset]}</span>";
		}		
		echo('<span class=smallprint><br/>This is the default character set used in the backend and the frontend. It may be overridden by individual sites and pages.</span>');
		echo('<span class=smallprint><br/><input type=checkbox name=check_db_mbstrings[] value=web '.((in_array('web',$this->check_db_mbstrings))?'checked':'').'> Validate data coming out of the <b>web</b> database is in this character set and uncorrupted.<br/><input type=checkbox name=check_db_mbstrings[] value=users '.((in_array('users',$this->check_db_mbstrings))?'checked':'').'> Validate data coming out of the <b>users</b> database is in this character set and uncorrupted.</span>');
		echo('<span class=warning style="font-size: 10px;"><br/><b>Warning: These settings are only necessary for multi-byte character sets.</b> You must have the <i>mbstring</i> module for PHP installed for these two options to work correctly. Only a few character sets are supported (generally Japanese ones).</span>');

		$backend->open_field('Site Backend DHTML Navigation','one_liner');
		if (superuser('web')) {
			echo combo_box('site_backend_dhtml_navigation',array('1'=>'On','0'=>'Off'),$this->site_backend_dhtml_navigation);
		} else {
			echo (($this->site_backend_dhtml_navigation)?'On':'Off');
		}

*/
/*
		 ##############
		# web-masters
		if (superuser()) {
			if (count($webmasters)) {
				$backend->open_field('Current Web Masters ('.count($webmasters).'):', 'top');
			
				$sql = "SELECT userid, login, CONCAT(firstname, ' ', surname) as name, email
						FROM user
						WHERE login IN ('".implode("','", $webmasters)."')
						ORDER BY surname, firstname";

				$webmasters = $db->associative_array($sql);

				$users_system->display_users_table(array_keys($webmasters), 'delete_webmaster_ids','Delete?','web_page_counter');
			}#end if count(webmasters)
			$backend->open_field('Add Web Masters:<br/><span class=smallprint>(logins/emails)</span>',  'top');
			echo text_area(webmaster_search,'',30,3,3000,'class=backend_data');
		}#end if superuser
*/

		if (true || superuser()) {
			$o->openSection('Packages');
			$o->openField('List', 'top');
			//$XTRAS->paintBackend($backend);
		}
	
		if(true || superuser('web')) {
			$backend->print_commit_button('Commit',"if (confirm('Commit this information to the system?')) { document.edit.action.value='Commit';document.edit.submit(); }");
		}

	}//end print_backend()

}//end class

?>
