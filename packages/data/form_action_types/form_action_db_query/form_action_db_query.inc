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
* $Id: form_action_db_query.inc,v 1.9 2012/12/20 03:56:14 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';

/**
* Form_Action_DB_Query
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage data
*/


class Form_Action_DB_Query extends Form_Action
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		self::_fillDefaults($settings);
		?>
		<script type="text/javascript">
			updateDbMethod = function(form) {
				method_db = form.<?php echo $prefix ?>_method_db;

				el_list = document.getElementById("<?php echo $prefix ?>_conn_details_div").getElementsByTagName("select");
				for (i = 0; i < el_list.length; i++) {
					el_list[i].disabled = !method_db[0].checked;
				}

				el_list = document.getElementById("<?php echo $prefix ?>_conn_details_div").getElementsByTagName("input");
				for (i = 0; i < el_list.length; i++) {
					el_list[i].disabled = !method_db[0].checked;
				}

				el_list = document.getElementById("<?php echo $prefix ?>_conn_string_div").getElementsByTagName("input");
				for (i = 0; i < el_list.length; i++) {
					el_list[i].disabled = !method_db[1].checked;
				}

				// Disable user name/password section id using a DB Connector
				el_list = document.getElementById("<?php echo $prefix ?>_login_section").getElementsByTagName("input");
				for (i = 0; i < el_list.length; i++) {
					el_list[i].disabled = method_db[2].checked;
				}

				el_list = document.getElementById("<?php echo $prefix ?>_connector_div").getElementsByTagName("input");
				for (i = 0; i < el_list.length; i++) {
					// If we are moving off the DB Connector
					if ((el_list[i].type == 'button') && (el_list[i].value == 'Cancel')) {
						el_list[i].click();
					}
					el_list[i].disabled = !method_db[2].checked;
				}
			}
		</script>
		<?php $o->addOnLoad('updateDbMethod(document.main_form);'); ?>

		<p class="sq-backend-section-subheading">Database Details</p>

		<div id="<?php echo $prefix ?>_dsn_section" style="padding: 0.5em 0px 2em 30px;">

		<p><?php
			radio_button($prefix.'_method_db', 'table', ($settings['method'] == 'table') ? TRUE : FALSE, 'updateDbMethod(this.form); return true;');
		?><strong>Specify database details</strong></p>

		<div id="<?php echo $prefix ?>_conn_details_div" style="padding-left: 30px">

		<?php
		// Or use custom DB connection
		$db_type  = array_get_index($settings, 'db_type', '');
		$database = array_get_index($settings, 'database', '');
		$host     = array_get_index($settings, 'host', '');

		?>
		<table class="sq-backend-table" style="width:auto">
			<tr>
				<td><?php echo translate('Database Type');?></td>

				<td>
					<?php
					$options = Array('pgsql' => 'PostgreSQL', 'oci' => 'Oracle', 'mysql' => 'MySQL');
					combo_box($prefix.'_dsn[db_type]', $options, FALSE, $db_type);
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('Database Name');?></td>

				<td>
					<?php
						text_box($prefix.'_dsn[database]', $database, 35);
					?>
				</td>
			</tr>
			<tr>
				<td><?php echo translate('Host Name');?></td>

				<td>
					<?php
						text_box($prefix.'_dsn[host]', $host, 35);
					?>
				</td>
			</tr>
		</table>
		<?php

		// Or use connection string
		?>
			</div>

		<p><?php
			radio_button($prefix.'_method_db', 'string', ($settings['method'] == 'string') ? TRUE : FALSE, 'updateDbMethod(this.form); return true;');
		?><strong>Use a complete DSN</strong></p>
			<div id="<?php echo $prefix ?>_conn_string_div" style="padding-left: 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td><p>Complete DSN</p></td>
					<td><p><?php text_box($prefix.'_complete_dsn', ($settings['method'] == 'string') ? $settings['dsn'] : '', 70); ?></p></td>
				</tr>
			</table>
			</div>

		<?php
		// Or use DB Connector asset
		?><p><?php
			radio_button($prefix.'_method_db', 'connector', ($settings['method'] == 'connector') ? TRUE : FALSE, 'updateDbMethod(this.form); return true;');
		?> <strong>Use the contents of a DB Connector asset</strong></p><?php

		?>
		<div id="<?php echo $prefix ?>_connector_div" style="padding-left: 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td><p>DB Connector Asset</p></td>
					<td><p><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('db_connector'=>'D')); ?><br/>
					Note: If a DB Connector asset is used, the user name and password fields below will be ignored, and the settings from that asset used instead.</p></td>
				</tr>
			</table>
		</div>

		</div>

		<p class="sq-backend-section-subheading">Login Details</p>

		<div id="<?php echo $prefix ?>_login_section" style="padding: 0.5em 0px 2em 30px;">
		<table style="width:auto">
			<tr>
				<td><strong><?php echo translate('User Name');?></strong></td>

				<td>
					<?php
						text_box($prefix.'_username', $settings['username'], 35);
					?>
				</td>
			</tr>
			<tr>
				<td><strong><?php echo translate('Password');?></strong></td>

				<td>
					<?php
						password_box($prefix.'_password', $settings['password'], 35);
					?>
				</td>
			</tr>
		</table>

		</div>

		<p class="sq-backend-section-subheading">SQL Query</p>

		<div id="<?php echo $prefix ?>_query_section" style="padding: 0.5em 0px 2em 30px;"><?php

		// Query field
		text_area($prefix.'_sql', $settings['sql'], 100, 40, 0, 'style="width: 80%; height: 30em; font-family: \'Courier New\', Courier, monospace; display: block"');

		?></div><?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		$settings = Array();

		$settings['method'] = array_get_index($_POST, $prefix.'_method_db', '');

		switch(array_get_index($_POST, $prefix.'_method_db', '')) {
			case 'table':
				$settings['dsn'] =  $_POST[$prefix.'_dsn']['db_type'].':';
				if (array_get_index($_POST[$prefix.'_dsn'], 'host', '') !== '') {
					$settings['dsn'] .= 'host='.$_POST[$prefix.'_dsn']['host'].';';
				}
				if (array_get_index($_POST[$prefix.'_dsn'], 'database', '') !== '') {
					$settings['dsn'] .= 'dbname='.$_POST[$prefix.'_dsn']['database'].';';
				}

				$settings['dsn'] = trim($settings['dsn'], ';');

				$settings['db_type']  = array_get_index($_POST[$prefix.'_dsn'], 'db_type', '');
				$settings['database'] = array_get_index($_POST[$prefix.'_dsn'], 'database', '');
				$settings['host']     = array_get_index($_POST[$prefix.'_dsn'], 'host', '');

				$settings['username'] = array_get_index($_POST, $prefix.'_username', '');
				$settings['password'] = array_get_index($_POST, $prefix.'_password', '');
			break;

			case 'string':
				$settings['dsn'] = $_POST[$prefix.'_complete_dsn'];
				$settings['username'] = array_get_index($_POST, $prefix.'_username', '');
				$settings['password'] = array_get_index($_POST, $prefix.'_password', '');
			break;

			case 'connector':
				$settings['connector_assetid'] = $_POST[$prefix.'_connector']['assetid'];
			break;
		}

		$settings['sql']      = array_get_index($_POST, $prefix.'_sql', '');

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		self::_fillDefaults($settings);

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Data Source:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['dsn'])) {
						echo htmlspecialchars($settings['dsn']);
					} else if (!empty($settings['connector_assetid'])) {
						echo get_asset_tag_line($settings['connector_assetid']);
					} else {
						?><span class="sq-backend-warning">No data source specified.</span><?php
					}
					?></p></td>
				</tr><?php
				if (!empty($settings['dsn'])) {
				?><tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>User Name:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
						if (!empty($settings['username'])) {
							echo htmlspecialchars($settings['username']);
							if (!empty($settings['password'])) {
								?> <em>(password supplied)</em><?php
							}
						} else {
							?>No user name specified.<?php
						}
					?></p></td>
				</tr><?php
				}
				?><tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>SQL Query:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['sql'])) {
						echo ellipsisize(preg_replace('/\\n/', ' ', htmlspecialchars($settings['sql'])), 512);
					} else {
						?><span class="sq-backend-warning">No SQL query specified.</span><?php
					} ?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	*/
	public static function execute(Form $form, $settings)
	{
		$result = Array();

		$db = self::_connectToDB($form, $settings);
		if ($db === FALSE) return FALSE;
		$sql = trim($settings['sql'], ';');

		$found = 0;

		// We will replace all responses, including raw values.
		// We will also replace keywords relating to the form submission asset
		// (eg. submission assetid, time, IP address)
		$matches = Array();
		$found = preg_match_all('/%(response_(\\d+_)?q\\d+(_raw)?)%/U', $sql, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(form_submission_.*)%/U', $sql, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);

		$i = 1;
		$bind_vars = Array();
		foreach ($matches as $match) {
			if (empty($match)) continue;
			$count = 0;
			do {
				$sql = preg_replace('/%'.$match[1].'%/U', ':response_'.$i, $sql, 1, $count);
				if ($count > 0) {
					$bind_vars['response_'.$i] = html_entity_decode($form->_getThankYouKeywordReplacement($match[1]));
					$i++;
				}
			} while ($count > 0);
		}

		try {
			MatrixDAL::changeDb('form_action_db_query_'.$form->id);
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			MatrixDAL::execPdoQuery($query);
			MatrixDAL::restoreDb();
		} catch (Exception $e) {
			trigger_localised_error('DATA0001', E_USER_WARNING, $e->getMessage(), $sql);
			return $result;
		}

		return TRUE;

	}//end execute()


	/**
	* Execute form action
	*
	*/
	public static function isValid(Form $form, $settings)
	{
		self::_fillDefaults($settings);

		// Must have either a DSN or a DB connector to be valid
		if (empty($settings['dsn']) && empty($settings['connector_assetid'])) {
			return FALSE;
		}

		// SQL must also be filled in
		if (empty($settings['sql'])) {
			return FALSE;
		}

		return TRUE;

	}//end isValid()


	/**
	* Fill any missing values from settings
	*
	* @param array	&$settings	Action settings
	*
	* @return void
	*/
	private static function _fillDefaults(&$settings) {
		if (!isset($settings['method'])) {
			$settings['method'] = 'table';
		}

		if (!isset($settings['db_type'])) {
			$settings['db_type'] = '';
		}

		if (!isset($settings['database'])) {
			$settings['database'] = '';
		}

		if (!isset($settings['host'])) {
			$settings['host'] = '';
		}

		if (!isset($settings['dsn'])) {
			$settings['dsn'] = '';
		}

		if (!isset($settings['username'])) {
			$settings['username'] = '';
		}

		if (!isset($settings['password'])) {
			$settings['password'] = '';
		}

		if (!isset($settings['sql'])) {
			$settings['sql'] = '';
		}

		if (!isset($settings['connector_assetid'])) {
			$settings['connector_assetid'] = 0;
		}

	}//end _fillDefaults()


	/**
	* Function to connect to the db and return the db object
	*
	* @return object
	*/
	private static function _connectToDB(Form $form, $settings)
	{
		if (!empty($settings['dsn'])) {
			$dsn_string = $settings['dsn'];
			$username   = $settings['username'];
			$password   = $settings['password'];

			$db_type = substr($dsn_string, 0, strpos($dsn_string, ':'));

			// If there is an "oci:" prefix, strip it and store this data in the DB type index
			if (substr($dsn_string, 0, 4) == 'oci:') {
				$dsn_string = substr($dsn_string, 4);
				// Also we may have arrived from the separate fields interface or manually entered a "dbname=" PDO_OCI-type DSN
				// In such cases we will be nice and strip the leading "dbname=" for a simple PHP OCI-friendly format
				if (substr($dsn_string, 0, 7) == 'dbname=') {
					$dsn_string = substr($dsn_string, 7);
				}
			}

			$dsn = Array(
					'DSN'		=> $dsn_string,
					'user'		=> $username,
					'password'	=> $password,
					'type'		=> $db_type,
			   );

		} else if (!empty($settings['connector_assetid'])) {
			$connector	= $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
			$dsn	 	= $connector->getDSN();
		} else {
			return FALSE;
		}

		$db = MatrixDAL::dbConnect($dsn, 'form_action_db_query_'.$form->id);
		return $db;

	}//end connectToDB()


}//end class

?>
