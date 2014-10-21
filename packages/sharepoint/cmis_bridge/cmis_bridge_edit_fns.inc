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
* $Id: cmis_bridge_edit_fns.inc,v 1.9 2012/08/30 00:59:19 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* CMIS_Bridge_Edit_Fns
*
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class CMIS_Bridge_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for editing WSDL URLs
 	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	*/
	function paintWsdlUrl(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$wsdl = $asset->attr('wsdl');

		?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							<?php echo 'Service Name'; ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo 'WSDL URL'; ?>
						</td>
						<td class="sq-backend-table-header">
							<?php echo 'Connection Status'; ?>
						</td>
					</tr>
				<?php

				foreach ($wsdl as $service => $url) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php echo $service; ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								if ($write_access){
									text_box($prefix.'[wsdl_urls]['.$service.']', $url, '100');
								} else {
									echo $url;
								}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
								$conn = $asset->connect($service, TRUE);
								if ($conn === FALSE) {
									echo '<span style="color: red"><b>'.translate('Not Connected').'</b></span>';

								} else {
									echo '<span style="color: green"><b>'.translate('Connected').'</b></span>';

								}
							?>
						</td>
					</tr>
					<?php
				}//end foreach data
				?>
				</table>
		<?php
		return TRUE;

	}//end paintWsdlUrl()


	/**
	* Process the interface for editing WSDL URLs
	*
	* @param object $asset
	* @param object $o
	* @param string $prefix
	* @return boolean
	 */
	function processWsdlUrl(&$asset, &$o, $prefix)
	{
        if (!$asset->writeAccess('attributes')) return FALSE;

        if (isset($_POST[$prefix]['wsdl_urls'])) {
	        $wsdl = $_POST[$prefix]['wsdl_urls'];
	        foreach ($wsdl as $service => $url){
	        	$wsdl[$service] = trim($url);
	        }

			$asset->setAttrValue('wsdl', $wsdl);
			$asset->saveAttributes();
        }

		return TRUE;

	}//end processWsdlUrl()


	/**
	* paint function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintSynchCheck(&$asset, &$o, $prefix)
	{
		$wa	= $asset->writeAccess('attributes');
		if ($wa) check_box($prefix.'_synch');
		return TRUE;

	}//end paintSynchCheck()


	/**
	* process function for synching
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processSynchCheck(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_synch'])) {
			$vars['bridge_id'] = $asset->id;
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_sync_cmis_bridge', $vars, '', SQ_PACKAGES_PATH.'/sharepoint/hipo_jobs/');
		}

		return TRUE;
	}//end processSynchCheck()


	/**
	* Paint the inteface for configuring metadata-fields used for synchronization
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function paintMetadataOptions(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$current_metadata_fields = $asset->attr('metadata_fields');
		$metadata_fields = $asset->getAvailableDocumentFields();
		if (empty($metadata_fields)) return FALSE; //should never be empty unless !$this->isConnected()

			?>
			<table class="sq-backend-table">
				<tr>
					<th><?php echo "Field"; ?></th>
					<th><?php echo "Field Type"; ?></th>
					<th><?php echo "Metadata Field ID" ?></th>
				</tr>
			<?php
			foreach ($metadata_fields as $property => $info) {
				?>
				<tr>
					<td>
						<?php
							echo $info['display_name'];
						?>
					</td>
					<td>
						<?php
							echo $info['type'];
						?>
					</td>
					<td>
						<?php
						$field_id = isset($current_metadata_fields[$property]['field']) ? $current_metadata_fields[$property]['field'] : '';
						if ($wa) {
							$type_codes = Array('metadata_field' => 'D');
							if ($property == 'cmis:objectId') $type_codes = Array('metadata_field_text' => 'I');
							if ($property == 'cmis:lastModificationDate') $type_codes = Array('metadata_field_date' => 'I');
							asset_finder($prefix.'_'.$property, $field_id, $type_codes);
						} else {
							if ($field_id) echo get_asset_tag_line($field_id);
						}
						?>
					</td>
				</tr>
				<?php
			}//end foreach
			?>
			</table>
			<?php
		return TRUE;

	}//end paintMetadataOptions()


	/**
	* Process the inteface for configuring metadata-fields used for synchronization
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access private
	*/
	function processMetadataOptions(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$applied_schemas = $mm->getSchemas($asset->id, TRUE, TRUE);
		$fields = $mm->getMetadataFields($applied_schemas);

		$current_metadata_fields = $asset->attr('metadata_fields');
        $metadata_fields = $asset->getAvailableDocumentFields();
        if (empty($metadata_fields)) return FALSE; //should never be empty unless !$this->isConnected()

        foreach ($metadata_fields as $property => $info){
        	 $current_field_id = isset($current_metadata_fields[$property]['field']) ? $current_metadata_fields[$property]['field'] : '';
        	 $field_id = (isset($_POST[$prefix.'_'.$property]['assetid'])) ? $_POST[$prefix.'_'.$property]['assetid'] : $current_field_id;
        	 if (!empty($field_id)){
        	 	if (!array_key_exists($field_id, $fields)){
        	 		trigger_localised_error('SHAR0001', sprintf(translate('Metadata field (Id: #%s) must belong to a schema applied to the bridge with cascades on.'), $field_id), E_USER_WARNING);
        	 		//revert to original value
        	 		$metadata_fields[$property]['field'] = $current_field_id;
        	 		continue;
        	 	}
        	 	if ($property == 'cmis:objectId'){
        	 		$type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($field_id, Array('metadata_field_text'));
        	 		if (empty($type_info)) {
        	 			trigger_localised_error('SHAR0004', sprintf(translate('Field \'%1$s\' must be a \'%2$s\'.'), $info['display_name'], 'Metadata Text Field'), E_USER_WARNING);
        	 			//revert to original value
        	 			$field_id = $current_field_id;
        	 		}
        	 	} else if ($property == 'cmis:lastModificationDate'){
        	 		 $type_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeInfo($field_id, Array('metadata_field_date'));
        	 		 if (empty($type_info)) {
        	 		 	trigger_localised_error('SHAR0004', sprintf(translate('Field \'%1$s\' must be a \'%2$s\'.'), $info['display_name'], 'Metadata Date Field'), E_USER_WARNING);
        	 		 	//revert to original value
        	 			$field_id = $current_field_id;
        	 		 }
        	 	}
        	 }

        	 $metadata_fields[$property]['field'] = $field_id;
        }

		$asset->setAttrValue('metadata_fields', $metadata_fields);
		$asset->saveAttributes();

		return TRUE;

	}//end processMetadataOptions()


	/**
	* Show-if conditional: display metadata options if connected and doc library found.
	*
	* @param object &$asset asset being painted
	*
	* @return boolean
	* @access public
	*/
	function isConnected(&$asset)
	{
		foreach ($asset->attr('wsdl') as $service_name => $url){
			$connected = $asset->connect($service_name, TRUE);
			if (!$connected) break;
		}
		if (empty($connected)) return FALSE;

		$repositoryId = $asset->getRepositoryId();
		if (empty($repositoryId)) return FALSE;

		return TRUE;

	}//end isConnected()



}//end class
?>
