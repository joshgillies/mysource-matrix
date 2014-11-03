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
*
*/


/**
* System Maintenance
*
* Purpose
*	Manages System Maintenance Mode and generate system reports
*
*
* @package MySource_Matrix
*/
class System_Maintenance_Config extends Config
{




	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars;


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/maintenance.inc';
		$this->config_vars = Array(
						'SQ_MAINTENANCE_MODE'				=> Array('editable' => 1, 'default' => 0),
						'SQ_MAINTENANCE_ASSET_TYPE'			=> Array('editable' => 1, 'default' => 'page_asset_builder:1|page_custom_form:1|page_account_manager:1|ecommerce_checkout:1|page_password_reset:1|page_online_quiz:1|page_link_manager:1'),
						'SQ_MAINTENANCE_ROOT_NODE'			=> Array('editable' => 1, 'default' => '1:1'),
						'SQ_MAINTENANCE_EXCLUDE_ROOT_NODE'			=> Array('editable' => 1, 'default' => ''),
						'SQ_MAINTENANCE_FRONTEND_NOTIFY_MESSAGE'	=> Array('editable' => 1, 'default' => translate('This website will soon be undergoing maintenance. Some website functions will temporarily be disabled.')),

						'SQ_MAINTENANCE_BACKEND_NOTIFY_MESSAGE'	=> Array('editable' => 1, 'default' => translate('This Matrix system will soon be entering Maintenance Mode. Please save your work and consult the system owner for further information.')),

						'SQ_MAINTENANCE_FRONTEND_ENABLED_MESSAGE'	=> Array('editable' => 1, 'default' => translate('This website is currently undergoing maintenance. Some website functions are temporarily disabled.')),

						'SQ_MAINTENANCE_BACKEND_ENABLED_MESSAGE'	=> Array('editable' => 1, 'default' => translate('This Matrix system has entered Maintenance Mode. Please consult the system owner before making any changes.')),

						'SQ_MAINTENANCE_RETRY_AFTER'			=> Array('editable' => 1, 'default' => 3600),
					   );
		$this->name = translate('System Maintenance');

	}//end constructor


	/**
	* Rewrites the conf file with current variables
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

		return parent::save($vars, $backup_existing, $send_message);

	}//end save()



	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end canAcquireLock()

	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!parent::writeAccess()) return FALSE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end writeAccess()


	/**
	* Manages display and processing of System Maintenance reports
	*
	* @param object	&$backend	Reference to backend object
	*
	* @return void
	* @access public
	*/
	public function paintBackend(&$backend)
	{
		require_once SQ_ATTRIBUTES_PATH.'/wysiwyg/wysiwyg.inc';

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		$o =& $backend->out;


		// if we dont have access, go away
		if (!$is_admin) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('Login'), translate('You do not have permission to access the asset'));


			exit();
		}

		if (isset($_POST['sq_lock_acquire']) || isset($_POST['process_form'])) {
			$this->processBackend($o);
		}

		$lock = $this->getLockInfo();
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);
		$write_access = $this->writeAccess();
		$class = get_class_lower($this);


		parent::paintBackend($o);

		if (isset($_GET['report_type_code'])) {
			$report = $_GET['report_type_code'];

			$this->_processReport($o, $report);
			$this->_paintReport($o, $report);
			exit;
		}


		$o->setHeading(translate('System Maintenance'), sq_get_icon($o->filesPath('/images/icons/header/system_maintenance.png'), '', '', translate('System Maintenance')));

		$o->setPageTitle(translate('System Maintenance'));


		/*
		*	maintenance mode
		*/
		$o->openSection(translate('Maintenance Mode'));

		$o->openField(translate('Maintenance Mode'), '', translate('When this setting is set to "Enabled", Matrix will enter Maintenance Mode and affected assets specified below will print a maintenance message instead of their asset contents on the front end.  If this setting is set to "Notify", Matrix will only print a different maintenance message as well as the asset contents on the front end for the affected assets. The %globals_system_maintenance_mode% keyword can be used to print the current maintenance mode status which will either be "disabled", "notify" or "enabled".'));


		if ($write_access && $is_admin) {
			combo_box($class.'[SQ_MAINTENANCE_MODE]', Array('0' => translate('Disabled'), '1' => translate('Notify'), '2' => translate('Enabled')), FALSE, SQ_MAINTENANCE_MODE);



		} else {
			switch (SQ_MAINTENANCE_MODE) {
				case '0':
					echo translate('Disabled');

					break;
				case '1':
					echo translate('Notify');

					break;
				case '2':
					echo translate('Enabled');

					break;
			}
		}
		$o->closeField();

		$o->openField(translate('Back End Notify Message'), '', translate('The message that will be printed in Admin and Edit mode when Maintenance Mode is set to "Notify" status. The <strong>%globals_system_maintenance_message_backend%</strong> keyword can also be used to print this message manually but will print blank when Maintenance Mode is disabled.'));


			$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_BACKEND_NOTIFY_MESSAGE';
			$wysiwyg = new Asset_Attribute_WYSIWYG();
			$wysiwyg->value = SQ_MAINTENANCE_BACKEND_NOTIFY_MESSAGE;
			$wysiwyg->paint($wysiwyg_prefix, !$write_access);
		$o->closeField();

		$o->openField(translate('Front End Notify Message'), '', translate('The message that will be printed on affected asset on frontend when Maintenance Mode is set "Notify" status. The <strong>%globals_system_maintenance_message_frontend%</strong> keyword can also be used to print this message manually but will print blank when Maintenance Mode is disabled.'));


			$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_FRONTEND_NOTIFY_MESSAGE';
			$wysiwyg = new Asset_Attribute_WYSIWYG();
			$wysiwyg->value = SQ_MAINTENANCE_FRONTEND_NOTIFY_MESSAGE;
			$wysiwyg->paint($wysiwyg_prefix, !$write_access);
		$o->closeField();

		$o->openField(translate('Back End Enabled Message'), '', translate('The message that will be printed in Admin and Edit mode when Maintenance Mode is set to "Enabled" status. The <strong>%globals_system_maintenance_message_backend%</strong> keyword can also be used to print this message manually but will print blank when Maintenance Mode is disabled.'));


			$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_BACKEND_ENABLED_MESSAGE';
			$wysiwyg = new Asset_Attribute_WYSIWYG();
			$wysiwyg->value = SQ_MAINTENANCE_BACKEND_ENABLED_MESSAGE;
			$wysiwyg->paint($wysiwyg_prefix, !$write_access);
		$o->closeField();

		$o->openField(translate('Front End Enabled Message'), '', translate('The message that will be printed on affected asset on frontend when Maintenance Mode is set to "Enabled" status. The <strong>%globals_system_maintenance_message_frontend%</strong> keyword can also be used to print this message manually but will print blank when Maintenance Mode is disabled.'));


			$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_FRONTEND_ENABLED_MESSAGE';
			$wysiwyg = new Asset_Attribute_WYSIWYG();
			$wysiwyg->value = SQ_MAINTENANCE_FRONTEND_ENABLED_MESSAGE;
			$wysiwyg->paint($wysiwyg_prefix, !$write_access);
		$o->closeField();


		$o->openField(translate('Affected Asset Types'), '', translate('When Maintenance Mode is enabled, the above types of assets will be affected.'));


			$this->paintAssetTypes($o, $class);
		$o->closeField();

		$o->openField(translate('Affected Root Nodes'), '', translate('When Maintenance Mode is enabled, the affected assets under the above root nodes will print the <strong>Front End Message</strong> instead of printing their content.'));


			$this->paintRootNode($o, $class);
		$o->closeField();

		$o->openField(translate('Excluded Root Nodes'), '', translate('The above root nodes are excluded and will overwrite any root nodes configured to be affected by Maintenance Mode.'));


			$this->paintExcludeRootNode($o, $class);
		$o->closeField();

		$o->openField(translate('Retry-After Response Header Value'), '', translate('When Maintenance Mode is enabled, affected assets will send a 503 Service Unavailable and a Retry-After response header to the requesting client to indicate that the page is unavailable and to try again after a certain amount of time. The value of this field can be either an HTTP-date or an integer number of seconds (in decimal) after the time of the response.'));


			if ($write_access && $is_admin) {
				text_box($class.'[SQ_MAINTENANCE_RETRY_AFTER]', SQ_MAINTENANCE_RETRY_AFTER, 40);
			} else {
				echo SQ_MAINTENANCE_RETRY_AFTER;
			}
		$o->closeField();

		$o->closeSection();




		$o->openSection(translate('Reports'));

		$o->openField('');
		$reports = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('simple_report');

		if (count($reports) > 0) {
			$backend_url = $backend->getBackendUrl('main');

			?>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script>
			<script type="text/javascript">
				var selectedReport = '';

				function toggleDiv(id)
				{
					var d = document.getElementById(id);
					var disp = "none";

					if (d.style.display == "none") {
						disp = "block";
					}

					d.style.display = disp;
				}


				function regenCallback(responseText)
				{
					document.getElementById(selectedReport).innerHTML = responseText;
					bg = document.getElementById(selectedReport+"_regen").style.background;
					document.getElementById(selectedReport+"_regen").style.background = "#e8ddeb";
					window.setTimeout('document.getElementById(selectedReport+"_regen").style.background = bg;', 1500);
				}


				function regen(report)
				{
					selectedReport = report;
					var regenElement = document.getElementById(report+"_regen");
					var reportElement = document.getElementById(report+"_report");
					reportElement.style.opacity = "0.4";
					reportElement.style.filter = "alpha(opacity=40)";
					regenElement.innerHTML = "Regenerating...";
					JsHttpConnector.submitRequest('<?php echo $backend_url; ?>&backend_section=sys_maintenance&report_type_code='+report, regenCallback);
				}
			</script>

			<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Name'); ?></th>

					<th><?php echo translate('Description'); ?></th>

				</tr>
				<?php

		// Get report name and description and sort reports by name
				$report_list = Array();
				foreach ($reports as $report_type_code) {
					$report_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($report_type_code);
					$report_name = $report_info['name'];
					$report_list[$report_name] = Array(
						'type_code'	=> $report_type_code,
						'description'	=> $report_info['description'],
						);
				}

				ksort($report_list);
				reset($report_list);

				$expand_reports = '';
				if (isset($_REQUEST['expand'])) $expand_reports = $_REQUEST['expand'];

				foreach ($report_list as $report_name => $report_info) {
					?>
					<tr>
						<td class="sq-backend-table-cell" style="background: white;"><a href="#<?php echo $report_info['type_code']; ?>" onclick="toggleDiv('<?php echo $report_info['type_code']; ?>');"><?php echo $report_name; ?></a></td>
						<td class="sq-backend-table-cell" style="background: white;"><?php echo $report_info['description']; ?>
							<div id="<?php echo $report_info['type_code']; ?>" style="display: <?php echo (($report_info['type_code'] == 'report_'.$expand_reports)?'block':'none'); ?>">
								<?php

								$this->_paintReport($o, $report_info['type_code']);

								?>
							</div>
						</td>
					</tr>
					<?php
				}
				?>
			</table>
			<?php
		} else {
			echo translate('There are no General Reports installed');

		}
		$o->closeField();
		$o->closeSection();

		if ($have_lock) $o->commitButton('', true);

	}//end paintBackend()



/**
	* Saves the config settings that were submitted
	*
	* @param object	&$o	reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	* @see User::validatePassword()
	*/
	function processBackend(&$o)
	{
		$class = get_class_lower($this);
		require_once $this->config_file;

		// process asset type
		if (isset($_POST[$class]['SQ_MAINTENANCE_ASSET_TYPE'])) {
			$value = '';
			$settings = $_POST[$class]['SQ_MAINTENANCE_ASSET_TYPE'];
			if(isset($settings['type_code'])) {
				foreach ($settings['type_code'] as $index => $type) {
					if(!empty($type) && isset($settings['inherit'][$index])) {
						$delimeter = $index == 0 ? '' : '|';
						$value .= $delimeter.$type.':'.$settings['inherit'][$index];
					}
				}
			}
			$_POST[$class]['SQ_MAINTENANCE_ASSET_TYPE'] = $value;
		}

		// process root node
		if (isset($_POST[$class.'_SQ_MAINTENANCE_ROOT_NODE_root_node'])) {
			$value = '';
			$settings = $_POST[$class.'_SQ_MAINTENANCE_ROOT_NODE_root_node'];
			$cascade_settings = $_POST[$class.'_SQ_MAINTENANCE_ROOT_NODE_all_underneath'];
			if(!empty($settings)) {
				foreach ($settings as $index => $node) {
					if(isset($node['assetid']) && !empty($node['assetid'])) {
						$delimeter = $index == 0 ? '' : '|';
						$value .= $delimeter.$node['assetid'];
						$cascade = isset($cascade_settings[$index]) && $cascade_settings[$index] ? '1' : '0';
						$value .= ':'.$cascade;
					}
				}
			}
			$_POST[$class]['SQ_MAINTENANCE_ROOT_NODE'] = $value;
		}

		// process exclude root node
		if (isset($_POST[$class.'_SQ_MAINTENANCE_EXCLUDE_ROOT_NODE_root_node'])) {
			$value = '';
			$settings = $_POST[$class.'_SQ_MAINTENANCE_EXCLUDE_ROOT_NODE_root_node'];
			$cascade_settings = $_POST[$class.'_SQ_MAINTENANCE_EXCLUDE_ROOT_NODE_all_underneath'];
			if(!empty($settings)) {
				foreach ($settings as $index => $node) {
					if(isset($node['assetid']) && !empty($node['assetid'])) {
						$delimeter = $index == 0 ? '' : '|';
						$value .= $delimeter.$node['assetid'];
						$cascade = isset($cascade_settings[$index]) && $cascade_settings[$index] ? '1' : '0';
						$value .= ':'.$cascade;
					}
				}
			}

			$_POST[$class]['SQ_MAINTENANCE_EXCLUDE_ROOT_NODE'] = $value;
		}

		// process messages
		$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_BACKEND_NOTIFY_MESSAGE';
		$wysiwyg = new Asset_Attribute_WYSIWYG();
		$wysiwyg->process($wysiwyg_prefix);
		$content = $wysiwyg->value;
		$_POST[$class]['SQ_MAINTENANCE_BACKEND_NOTIFY_MESSAGE'] = $content;


		$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_FRONTEND_NOTIFY_MESSAGE';
		$wysiwyg = new Asset_Attribute_WYSIWYG();
		$wysiwyg->process($wysiwyg_prefix);
		$content = $wysiwyg->value;
		$_POST[$class]['SQ_MAINTENANCE_FRONTEND_NOTIFY_MESSAGE'] = $content;

		$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_BACKEND_ENABLED_MESSAGE';
		$wysiwyg = new Asset_Attribute_WYSIWYG();
		$wysiwyg->process($wysiwyg_prefix);
		$content = $wysiwyg->value;
		$_POST[$class]['SQ_MAINTENANCE_BACKEND_ENABLED_MESSAGE'] = $content;


		$wysiwyg_prefix = $class.'_SQ_MAINTENANCE_FRONTEND_ENABLED_MESSAGE';
		$wysiwyg = new Asset_Attribute_WYSIWYG();
		$wysiwyg->process($wysiwyg_prefix);
		$content = $wysiwyg->value;
		$_POST[$class]['SQ_MAINTENANCE_FRONTEND_ENABLED_MESSAGE'] = $content;



		return parent::processBackend($o);

	}//end processBackend()



	/**
	* Paints the interface of a report
	*
	* @param object	&$o			Reference to Outputter object
	* @param string	$report_type_code	Asset type code of the report
	*
	* @return void
	* @access private
	*/
	private function _paintReport(&$o, $report_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($report_type_code)) {
			$am->includeAsset($report_type_code);
			?>
			<br>
			<div id="<?php echo $report_type_code; ?>_report">
			<?php
				call_user_func_array(Array($report_type_code, 'paintReport'), Array($o, $report_type_code));
			?>
			</div>
			<br><br>
			<?php
		}

	}//end _paintReport()


	/**
	* Processes the interface of a report
	*
	* @param object	&$o			Reference to Outputter object
	* @param string	$report_type_code	Asset type code of the report
	*
	* @return void
	* @access private
	*/
	private function _processReport(&$o, $report_type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($report_type_code)) {
			$am->includeAsset($report_type_code);
			call_user_func_array(Array($report_type_code, 'processReport'), Array($o, $report_type_code));
		}

	}//end _processReport()


	/**
	* Paint the interface for choosing which asset types to be covered by maintenance mode
	*
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetTypes(&$o, $prefix)
	{
		$types = Array();
		foreach (explode('|', SQ_MAINTENANCE_ASSET_TYPE) as $type) {
			if(empty($type)) continue;
			$type_array = explode(':', $type);
			$types[$type_array[0]] = $type_array[1];
		}
		if ($this->writeAccess()) {
			// print the form fields
			$display_values = Array('type_code' => Array(), 'inherit' => Array());
			foreach ($types as $type => $inherit) {
				$display_values['type_code'][] = $type;
				$display_values['inherit'][] = $inherit;
			}
			asset_type_chooser($prefix.'[SQ_MAINTENANCE_ASSET_TYPE]', TRUE, $display_values, NULL, TRUE, TRUE);
			return TRUE;
		} else {
			// print the read-only version
			if (empty($types)) {
				echo '<p class="sq-backend-warning">'.translate('No asset types selected.').'</p>';

			} else {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
				?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Type') ?></th>

							<th style="width:340px;" ><?php echo translate('Inherit?') ?></th>

						</tr>
				<?php
				$inherit_image_path = sq_web_path('lib').'/web/images/';
				foreach ($types as $type => $inherit) {
					$inherit_image = $inherit_image_path.($inherit ? 'tick' : 'cross').'.gif';
					$inherit_alt = $inherit ? translate('Yes') : translate('No');


					?>
						<tr>
							<td><?php
								echo get_asset_type_icon($type);
								echo $type_names[$type];
							?></td>
							<td><img src="<?php echo $inherit_image; ?>" alt="<?php echo $inherit_alt; ?>" /> <strong><?php echo $inherit_alt; ?></strong></td>
						</tr>
					<?php
				}
				?>
					</table>
				<?php
			}
			return FALSE;
		}//end else - if write access to content

	}//end paintAssetTypes()



	/**
	* Paints the root node selection box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$o, $prefix)
	{
		// get root asset ids
		$root_nodes = Array();
		foreach (explode('|', SQ_MAINTENANCE_ROOT_NODE) as $node) {
			if(empty($node)) continue;
			$details = explode(':', $node);
			$cascade = isset($details[1]) && $details[1] == 1 ? 1 : 0;
			$root_nodes[$details[0]] = Array('all_underneath' => $cascade);
		}

		multiple_root_nodes_selector($prefix.'_SQ_MAINTENANCE_ROOT_NODE', $root_nodes, $this->writeAccess());

		return TRUE;

	}//end paintRootNode()


	/**
	* Paints the root node exclusion box
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeRootNode(&$o, $prefix)
	{
		// get root asset ids
		$root_nodes = Array();
		foreach (explode('|', SQ_MAINTENANCE_EXCLUDE_ROOT_NODE) as $node) {
			if(empty($node)) continue;
			$details = explode(':', $node);
			$cascade = isset($details[1]) && $details[1] == 1 ? 1 : 0;
			$root_nodes[$details[0]] = Array('all_underneath' => $cascade);
		}

		multiple_root_nodes_selector($prefix.'_SQ_MAINTENANCE_EXCLUDE_ROOT_NODE', $root_nodes, $this->writeAccess());

		return TRUE;

	}//end paintExcludeRootNode()




}//end class

?>
