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
* $Id: proxy_authentication_config.inc,v 1.21 2013/07/25 23:23:49 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Messaging_Service_Config
*
* Purpose
*
*    Looks after the creation and editing of the messaging service config
*
* @author  Sertan Danis <sdanis@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix
*/
class Proxy_Authentication_Config extends Config
{

	/**
	* The array of default messaging config settings
	* @var array
	*/
	var $config_vars = Array(
						// Proxy Options
						'SQ_PA_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_PA_HOSTNAME'			=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_PORT'				=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_USERNAME'			=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_PASSWORD'			=> Array('editable' => 1, 'default' => '', 'skip_mailing' => TRUE),
						'SQ_PA_ALTERNATIVES'		=> Array('editable' => 1, 'default' => 'a:0:{}'),
						'SQ_PROXY_URL_PORT_STRIP'	=> Array('editable' => 1, 'default' => FALSE),
						// SSL Accelerator Options
						'SQ_SSLA_ENABLED'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_SSLA_MATCH'				=> Array('editable' => 1, 'default' => 'ip'),
						'SQ_SSLA_IP'				=> Array('editable' => 1, 'default' => ''),
						'SQ_SSLA_HEADER_NAME'		=> Array('editable' => 1, 'default' => ''),
						'SQ_SSLA_HEADER_VALUE'		=> Array('editable' => 1, 'default' => ''),
						'SQ_SSLA_FORCE_SECURE'		=> Array('editable' => 1, 'default' => FALSE),
					   );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
		$this->name = translate('Proxy Configuration');

	}//end constructor


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	Backend_Outputter, reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class_lower($this);

		$o->openSection('Primary Proxy Configuration');
			$o->openField(translate('Proxy Status'));

			if (SQ_PA_ENABLED) {
				require_once(SQ_INCLUDE_PATH.'/general_occasional.inc');
				$socket = open_socket_connection('http://matrix.squiz.net', $errno, $errstr);
				if ($socket !== FALSE && !is_null($socket)) {
					$response = fread($socket, 128);
					if (preg_match('|407 proxy authentication required|i', $response)) {
						echo '<font color="red"><strong>'.translate('Failed (Proxy Authentication Failed)').'</strong></font>';

					} else {
						echo '<font color="green"><strong>'.translate('Passed').'</strong></font>';

					}
					fclose($socket);
				} else {
					echo '<strong class="sq-backend-warning">'.translate('Proxy Configuration Problem').'</font>';

				}
			} else {
				echo '<strong>'.translate('Disabled').'</font>';

			 }
			 $o->closeField();

			$o->openField(translate('Enable Proxy'));

			if ($write_access) {
				combo_box($class.'[SQ_PA_ENABLED]', Array(translate('No'), translate('Yes')), FALSE, SQ_PA_ENABLED);


			} else {
				echo (SQ_PA_ENABLED ? translate('Yes') : translate('No'));


			}
			$o->closeField();
			$o->openField(translate('Hostname'));

			if ($write_access) {
				text_box($class.'[SQ_PA_HOSTNAME]', SQ_PA_HOSTNAME, 25);
			} else {
				echo SQ_PA_HOSTNAME;
			}
			$o->closeField();
			$o->openField(translate('Port'));

			if ($write_access) {
				text_box($class.'[SQ_PA_PORT]', SQ_PA_PORT, 5);
			} else {
				echo SQ_PA_PORT;
			}
			$o->closeField();
			$o->openField(translate('Username'));

			if ($write_access) {
				text_box($class.'[SQ_PA_USERNAME]', SQ_PA_USERNAME, 25);
			} else {
				echo SQ_PA_USERNAME;
			}
			$o->closeField();
			$o->openField(translate('Password'));

			if ($write_access) {
				password_box($class.'[SQ_PA_PASSWORD]', base64_decode(SQ_PA_PASSWORD), 25);
			} else {
				if (strlen(SQ_PA_PASSWORD)) {
					echo translate('Password not shown');

				}
			}
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Proxy URL Manipulation'));


			$o->openField(translate('Ignore Port Number'));

			if ($write_access) {
				combo_box($class.'[SQ_PROXY_URL_PORT_STRIP]', Array(translate('No'), translate('Yes')), FALSE, SQ_PROXY_URL_PORT_STRIP);


			} else {
				echo (SQ_PROXY_URL_PORT_STRIP ? translate('Yes') : translate('No'));


			}
			$o->note(translate('Useful if you access Squiz Matrix using a specific port number (i.e. specified in the URL). If you choose to ignore port numbers, URLs printed by Squiz Matrix will not contain the port number by which the system is being accessed.'));

			$o->closeField();

		$o->closeSection();

		$o->openSection('Alternative Proxy Configuration');
		$o->openField('');
			$this->paintAlternativeProxies($o);
		$o->closeField();
		$o->sectionNote('Alternative proxies will be used if they are set to be Active, and their URL pattern matches the URL being requested through the proxy. Proxies are matched from top to bottom; only the first proxy that matches will be used.');
		$o->closeSection('');

		$o->openSection('SSL Accelerator Configuration');
			$o->openField(translate('Handling Enabled?'));

				if ($write_access) {
					combo_box($class.'[SQ_SSLA_ENABLED]', Array(translate('No'), translate('Yes')), FALSE, SQ_SSLA_ENABLED);


				} else {
					echo (SQ_SSLA_ENABLED ? translate('Yes') : translate('No'));


				}
				$o->note(translate('The following settings will only apply if this setting is set to "Yes".'));

			$o->closeField();

			$o->openField(translate('Match Method'));

				$options = Array(
							'ip'		=> translate('IP Address'),

							'header'	=> translate('Header'),

							'both'		=> translate('Both'),

						   );
				if ($write_access) {
					combo_box($class.'[SQ_SSLA_MATCH]', $options, FALSE, SQ_SSLA_MATCH);
				} else {
					echo $options[SQ_SSLA_MATCH];
				}
				$o->note(translate('Squiz Matrix can use two methods to determine whether a request has been made through an SSL accelerator: by matching the IP address of the proxy, or through a header injected by the proxy into the request made to the Squiz Matrix system. Either or both match methods can be used. If "Both" is selected, the system is not considered behind an accelerator unless both methods match.'));

			$o->closeField();

			$o->openField(translate('IP Address'));

				if ($write_access) {
					text_box($class.'[SQ_SSLA_IP]', SQ_SSLA_IP, 15);
				} else {
					echo SQ_SSLA_IP;
				}
				$o->note(translate('If IP address matching is turned on, Squiz Matrix will only consider itself behind an SSL accelerator if the request (from the accelerator proxy) is made from the entered IP address. Remember that this the IP address that this system sees the accelerator as. This may be an internal network address, rather than any public-facing IP address.'));

			$o->closeField();

			$o->openField(translate('Header Name'));

				if ($write_access) {
					text_box($class.'[SQ_SSLA_HEADER_NAME]', SQ_SSLA_HEADER_NAME, 15);
				} else {
					echo SQ_SSLA_HEADER_NAME;
				}
				$o->note(translate(''));

			$o->closeField();

			$o->openField(translate('Header Value'));

				if ($write_access) {
					text_box($class.'[SQ_SSLA_HEADER_VALUE]', SQ_SSLA_HEADER_VALUE, 15);
				} else {
					echo SQ_SSLA_HEADER_VALUE;
				}
				$o->note(translate(''));

			$o->closeField();

			$o->openField(translate('Force Secure?'));

				if ($write_access) {
					combo_box($class.'[SQ_SSLA_FORCE_SECURE]', Array(translate('No'), translate('Yes')), FALSE, SQ_SSLA_FORCE_SECURE);


				} else {
					echo (SQ_SSLA_FORCE_SECURE ? translate('Yes') : translate('No'));


				}
				$o->note(translate('When this is set to "Yes", and Squiz Matrix detects that it is running behind an SSL accelerator, it will always produce secure (https://) URLs, except where an asset has been set to force standard http:// URLs.'));

			$o->closeField();

			$o->sectionNote(translate('An SSL accelerator is a hardware or software proxy whose primary function is to handle the decoding of secure (HTTPS) requests. If Squiz Matrix is located behind one of these, the SSL acclerator will handle the SSL layer of the request, and then send a plain (HTTP) request to the Squiz Matrix system. This would normally mean that the system would use HTTP urls back, rather than the expected HTTPS. Squiz Matrix is able to detect the presence of the SSL accelerator through the above settings, and send links using HTTPS protocol as expected.'));

		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Paints alternative proxies
	*
	* @param Backend_Outputter	$o				The outputter class
	*
	* @return void
	*/
	function paintAlternativeProxies(Backend_Outputter $o)
	{
		$all_proxies = SQ_PA_ALTERNATIVES;
		if (trim($all_proxies) !== '') {
			$all_proxies = unserialize(SQ_PA_ALTERNATIVES);
		} else {
			$all_proxies = Array();
		}

		/*
		 * Pattern DOES/DOES NOT options, used for select field.
		 */
		$pattern_not_options = Array(
								'0'	=> 'does',
								'1'	=> 'does not',
							   );

		/*
		 * Pattern type options to be used by the select field.
		 * This array also used for gramatically-correct pattern type
		 * descriptions when a DOES NOT is in force.
		 * eg. "URL DOES NOT begin with ..."
		 */
		$pattern_type_options = Array(
								'equals'	=> 'equal',
								'begins'	=> 'begin with',
								'ends'		=> 'end with',
								'contains'	=> 'contain',
							   );

		/*
		 * Gramatically-correct pattern type descriptions for cases where a
		 * DOES NOT is not involved.
		 * eg. change "URL DOES equal ..." to "URL equals ...".
		 */
		$pattern_type_desc = Array(
								'equals'	=> 'equals',
								'begins'	=> 'begins with',
								'ends'		=> 'ends with',
								'contains'	=> 'contains',
							   );

		$write_access = $this->writeAccess();
		$class = get_class_lower($this);

		$i = 0;
		foreach ($all_proxies as $proxy_key => $proxy_details) {
			$i++;
			$o->openSection('Alternative Proxy '.$i);
				$o->openField('Hostname');
					if ($write_access) {
						echo text_box($class.'_alternative[proxy_url]['.$proxy_key.']', array_get_index($proxy_details, 'proxy_url', ''), 30);
						$o->note(translate('Leave hostname blank if you want to disable proxy use for this matching pattern.'));

					} else {
						echo (array_get_index($proxy_details, 'proxy_url', '') === '') ? '<i>' . translate('None') . '</i>' : array_get_index($proxy_details, 'proxy_url', '');

					}
				$o->closeField();
				$o->openField('Port');
					if ($write_access) {
						echo int_text_box($class.'_alternative[proxy_port]['.$proxy_key.']', array_get_index($proxy_details, 'proxy_port', ''), FALSE, 7, 0, 65535);
					} else {
						echo array_get_index($proxy_details, 'proxy_port', '');
					}
				$o->closeField();
				$o->openField('Username');
					if ($write_access) {
						echo text_box($class.'_alternative[username]['.$proxy_key.']', array_get_index($proxy_details, 'username', ''), 10);
					} else {
						echo array_get_index($proxy_details, 'username', '');
					}
				$o->closeField();
				$o->openField('Password');
					if ($write_access) {
						echo password_box($class.'_alternative[password]['.$proxy_key.']', base64_decode(array_get_index($proxy_details, 'password', '')), 10);
					} else {
						if (trim(array_get_index($proxy_details, 'password', '')) !== '') {
							?><em>Password not shown</em><?php
						}
					}
				$o->closeField();
				$o->openField('Pattern');
					if ($write_access) {
						echo 'URL ';
						echo combo_box($class.'_alternative[pattern_not]['.$proxy_key.']', $pattern_not_options, FALSE, array_get_index($proxy_details, 'pattern_not', '0'));
						echo ' ';
						echo combo_box($class.'_alternative[pattern_type]['.$proxy_key.']', $pattern_type_options, FALSE, array_get_index($proxy_details, 'pattern_type', 'equals'));
						echo ' ';
						text_box($class.'_alternative[pattern_url]['.$proxy_key.']', array_get_index($proxy_details, 'pattern_url', ''), 30);
					} else {
						echo translate('comparison'.(array_get_index($proxy_details, 'pattern_not', '0') ? '_not_' : '_').array_get_index($proxy_details, 'pattern_type', 'equals'), 'URL', '"'.array_get_index($proxy_details, 'pattern_url', '').'"');
					}
				$o->closeField();
				$o->openField(translate('Active?'));

					if ($write_access) {
						echo check_box($class.'_alternative[active]['.$proxy_key.']', 1, array_get_index($proxy_details, 'active', TRUE));
						echo label(translate('Keep this proxy active'), $class.'_alternative[active]['.$proxy_key.']');

					} else {
						echo array_get_index($proxy_details, 'active', TRUE);
					}
				$o->closeField();
				if ($write_access) {
					$o->openField(translate('Delete?'));

						echo check_box($class.'_alternative[delete]['.$proxy_key.']', 1, FALSE);
						echo label(translate('Delete this proxy'), $class.'_alternative[delete]['.$proxy_key.']');

					$o->closeField();
				}
			$o->closeSection();
		}

		if ($write_access) {
			$o->openSection('Add New Proxy');
				$o->openField('Add new proxy');
					echo check_box($class.'_alternative[add]', 1, FALSE);
					echo label('Add new alternative proxy', $class.'_alternative[add]');
				$o->closeField();
			$o->closeSection();
		}

	}//end paintAlternativeProxies()


	public function processBackend(Backend_Outputter $o)
	{
		parent::processBackend($o);

	}//end processInterface();


	public function processAlternativeProxies(&$vars)
	{
		if (defined('SQ_PA_ALTERNATIVES')) {
			$all_proxies = SQ_PA_ALTERNATIVES;
			if (trim($all_proxies) !== '') {
				$all_proxies = unserialize(SQ_PA_ALTERNATIVES);
			} else {
				$all_proxies = Array();
			}
		} else {
			$all_proxies = Array();
		}

		$class = get_class_lower($this);

		$alt_post = array_get_index($_POST, $class.'_alternative', Array());

		if (!empty($alt_post)) {
			$deletes = array_get_index($alt_post, 'delete', Array());
			foreach ($deletes as $key => $value) {
				unset($all_proxies[$key]);
			}

			$actives = array_get_index($alt_post, 'active', Array());
			foreach ($all_proxies as $key => $value) {
				$all_proxies[$key]['active'] = isset($actives[$key]);

				$all_proxies[$key]['proxy_url']    = $alt_post['proxy_url'][$key];
				$all_proxies[$key]['proxy_port']   = $alt_post['proxy_port'][$key];
				$all_proxies[$key]['username']     = $alt_post['username'][$key];
				$all_proxies[$key]['password']     = base64_encode($alt_post['password'][$key]);
				$all_proxies[$key]['pattern_not']  = $alt_post['pattern_not'][$key];
				$all_proxies[$key]['pattern_type'] = $alt_post['pattern_type'][$key];
				$all_proxies[$key]['pattern_url']    = $alt_post['pattern_url'][$key];
			}

		}

		$add = array_get_index($alt_post, 'add', 0);
		if ($add) {
			// stop Matrix from throwing PHP notice if the
			// $all_proxies array is empty
			if (!empty($all_proxies)) {
				$new_key = max(array_keys($all_proxies)) + 1;
			} else {
				$new_key = 1;
			}
			$all_proxies[$new_key] = Array();
		}

		$vars['SQ_PA_ALTERNATIVES'] = serialize($all_proxies);

	}//end processAlternativeProxies()


	/**
	* Rewrites the conf file with current variables
	*
	* Return value indicates whether the file was written
	*
	* @param array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean	$send_message		whether to send a message using the messaging system
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
	    	// attempt to load the config file to make sure we get any current settings
		if (file_exists($this->config_file)) {
			require_once $this->config_file;
		}

		$this->processAlternativeProxies($vars);

		// Compress SSLA IP if its IPv6
		if (!empty($vars['SQ_SSLA_IP']) && isIpV6($vars['SQ_SSLA_IP'])) {
			$vars['SQ_SSLA_IP'] = compressIPv6Address($vars['SQ_SSLA_IP']);
		}

		// encode password
		if (isset($vars['SQ_PA_PASSWORD']) && !empty($vars['SQ_PA_PASSWORD'])) {
			$vars['SQ_PA_PASSWORD'] = base64_encode($vars['SQ_PA_PASSWORD']);
		}

		return parent::save($vars, $backup_existing, $send_message);

	}//end save()


}//end class

?>
