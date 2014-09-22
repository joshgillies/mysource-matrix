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
* $Id: data_source_db_edit_fns.inc,v 1.42 2012/08/30 00:58:15 ewang Exp $
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
* @version $Revision: 1.42 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_DB_Edit_Fns extends Data_Source_Edit_Fns
{

	public $dsn = NULL;
	public $result = NULL;
	public $fields = Array();
	public $selected_fields = Array();
	public $group_by = Array();
	public $conn = NULL;


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


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
	function paintConnectorLink(&$asset, &$o, $prefix)
	{
		// get root links
		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'db_connector', FALSE, 'db_connector');
		if (empty($connector_link)) {
			$connector_asset_id = NULL;
		} else {
			$connector_asset_id = $connector_link['minorid'];
		}

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_connector_node', $connector_asset_id, Array('db_connector'=>'D'));
		} else {
			if (empty($connector_asset_id)) {
				echo 'No connector selected';
			} else {
				echo get_asset_tag_line($connector_asset_id);
			}
		}
		return TRUE;

	}//end paintConnectorLink()


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
	function processConnectorLink(&$asset, &$o, $prefix)
	{
		$new_connector_id = @$_REQUEST[$prefix.'_connector_node']['assetid'];

		if (!$asset->writeAccess('links')) {
			trigger_error('You do not have write access to change the connector link', E_USER_WARNING);
			return FALSE;
		}

		$current_connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'db_connector', FALSE, 'db_connector');
		$current_connector_asset_id = @$current_connector_link['minorid'];

		if (!empty($current_connector_link)) {
			// if the new connector id is not null and is equal to the current
			// connector asset id then return
			if ((!empty($new_connector_id)) && ($current_connector_asset_id === $new_connector_id)) {
				return TRUE;
			} else {
				// else means that we want to create a new link or we want to
				// clear the link
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_connector_link['linkid']);

				// if the new connector id is empty means we want to clear the
				// link we got nothing else to do so return TRUE
				if (empty($new_connector_id))return TRUE;
			}
		}

		if (!empty($new_connector_id)) {
			$new_connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_connector_id);
			$result_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_connector_asset, SQ_LINK_NOTICE, 'db_connector');

			if (empty($result_id)) {
			trigger_error('Could not link the connector to this asset', E_USER_WARNING);
			return FALSE;
		}

		}

		return TRUE;

	}//end processConnectorLink()


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
		$db_type = $asset->attr('db_type');
		$database = $asset->attr('database');
		$host = $asset->attr('host');
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Database Type');?></th>

				<td>
					<?php
					$options = Array('postgres' => 'PostgreSQL', 'oracle' => 'Oracle', 'mysql' => 'MySQL');
					if ($wa) {
						combo_box('dsn[db_type]', $options, FALSE, $db_type);
					} else {
						if (empty($db_type)) {
							?><em>Ignored due to Complete DSN entry</em><?php
						} else {
							echo $options[$db_type];
						}
					}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('Database Name');?></th>

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
				<th><?php echo translate('Host Name');?></th>

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
		</table>
		<?php
		if ($wa) {
			return TRUE;
		} else {
			return FALSE;
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
		$dsn = $asset->attr('dsn');
		$new_dsn_string = NULL;
		$new_dsn = $_POST['dsn'];
		if (!empty($_POST['dsn']['dsn'])) {
			$new_dsn_string = $_POST['dsn']['dsn'];
			$new_dsn['db_type'] = NULL;
			$new_dsn['host'] = NULL;
			$new_dsn['database'] = NULL;
		} else {
			switch ($new_dsn['db_type']) {
				case 'postgres':
					$new_dsn_string = "pgsql:";
				break;
				case 'mysql':
					$new_dsn_string = "mysql:";
				break;
				case 'oracle':
					$new_dsn_string = "oci:";
				break;
			}

			if ($new_dsn['db_type'] == 'oracle') {
				// PDO_OCI compatible DSN is either:
				// oci:dbname=[db]...
				// oci:dbname=[hostname]/[db]...

				// Let's provide a PDO_OCI-like DSN and work it for PHP OCI out when connecting
				$new_dsn_string .= 'dbname=';

				// Start with the hostname...
				if (!empty($new_dsn['host'])) {
					$new_dsn_string .= "$new_dsn[host]/";
				}

				// Unless that isn't supplied, in which case we will be using a tnsnames-defined DB only
				$new_dsn_string .= "$new_dsn[database]";
			} else {
				$new_dsn_string .= "dbname=$new_dsn[database];host=$new_dsn[host]";
			}
		}
		if (strcasecmp($dsn, $new_dsn_string) != 0) {
			foreach ($new_dsn as $key => $value) {
				$asset->setAttrValue($key, $value);
			}

			$asset->setAttrValue('dsn', $new_dsn_string);
			return TRUE;
		}
		return FALSE;

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
		$database = $asset->attr('database');
		if ($wa) {
			text_area('dsn[dsn]', empty($database) ? $dsn : '', 70, 5);
			return TRUE;
		} else {
			if (!empty($dsn)) echo $dsn;
			return FALSE;
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
		return FALSE;

	}//end processDSNAlone()


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
		$wa = $asset->writeAccess('attributes');
		$sql = $asset->attr('sql');

		if ($wa) {
			$dynamic_vars = $asset->attr('dynamic_vars');
			if (!empty($dynamic_vars)) {
				foreach ($dynamic_vars as $var => $value) {
					$keywords['%%'.$var.'%%'] = $var;
				}
				keyword_selector($keywords, $prefix.'_sql');
			}
			text_area($prefix.'_sql', $sql, 100, 40, 0, 'style="width: 100%; height: 30em; font-family: \'Courier New\', Courier, monospace; display: block"');
		} else {
			if (!empty($sql)) {
				echo nl2br($sql);
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
			return $asset->setAttrValue('sql', $_POST[$prefix.'_sql']);
		}

		return FALSE;

	}//end processQuerySoFar()


	/**
	* Paint Debug info for this asset
	*
	* @param object &$asset the asset
	* @param object &$o             the backend outputter
	* @param string $prefix prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDebugInfo(&$asset, &$o, $prefix)
	{
		if ($asset->connectToDB()) {
			echo '<div style="color: green"><b>Connected</b></div>';
		} else {
			echo '<div style="color: red"><b>Not connected</b></div>';
		}

		if (isset($asset->_tmp['db_conection_exception'])) {
			echo "<div>Connection failed: ".$asset->_tmp['db_conection_exception']->getMessage()."</div>";
		}

		return TRUE;

	}//end paintDebugInfo()


}//end class
?>
