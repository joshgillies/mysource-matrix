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
* $Id: checkout_action_save_xml.inc,v 1.1 2013/07/09 09:02:30 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ecommerce/checkout_action/checkout_action.inc';

/**
* Checkout_Action_Save_Xml
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Checkout_Action_Save_Xml extends Checkout_Action
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
	* Process inline interface
	*
	* @access public
	* @return boolean
	*/
	public static function paintInlineInterface(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}

		// Paint something here!
		?>
		<table class="no-borders" style="width:auto">
			<tr>
				<td>Location</td>
				<td>
					<?php
					$location = array_get_index($xml_settings, 'location', '');
					text_box($prefix.'_location', $location, 50);
					if (empty($location) || !is_dir($location) || !is_writable($location)) {
						?><br />
						<span class="sq-backend-warning"><?php echo translate('location_warning'); ?></span>
						<?php
					}//end if
					?>
				</td>
			</tr>
			<tr>
				<td>Include Transaction Reference</td>
				<td>
					<?php
					$key = array_get_index($xml_settings, 'include_trans_ref', '0');
					check_box($prefix.'_include_trans_ref', 1, $key);
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @access public
	* @return boolean
	*/
	public static function processInlineInterface(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}

		$xml_settings['location'] = array_get_index($_POST, $prefix.'_location', '');
		$xml_settings['include_trans_ref'] = array_get_index($_POST, $prefix.'_include_trans_ref', '0');

		$settings['save_xml'] = $xml_settings;

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @access public
	* @return void
	*/
	public static function paintSummary(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}
		self::_fillDefaultValues($xml_settings);

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Location:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($xml_settings['location'])) {
						echo ellipsisize($xml_settings['location'], 512);
					} else {
						?><span class="sq-backend-warning">No Location specified.</span><?php
					} ?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute checkout action
	*
	* @access public
	* @return boolean
	*/
	public static function execute(Ecommerce_Checkout $checkout, Ecommerce_Order $order, $settings)
	{
		$xml_settings = $settings['save_xml'];

		// Double check
		if (!self::isValid($checkout, $settings)) return FALSE;

		$location = $xml_settings['location'];
		$include_trans_ref = $xml_settings['include_trans_ref'];
		$file = $location.'/order-'.$order->id.'-'.date('YmdHis', $order->created).'.xml';

        // Get order xml data
        $order_xml = $order->attr('order_xml');
        // If "strip HTML tags" global preference is enabled,
        // then we will need to decode the encoded xml data to get proper xml
        if (strpos($order_xml, '<') === FALSE) {
            $order_xml = htmlspecialchars_decode($order_xml, ENT_COMPAT);
        }

		// Include transaction reference details if required
		if ($include_trans_ref) {
			$sxml = @simplexml_load_string($order_xml);
			if ($sxml) {
				$transaction_node = $sxml->addChild('transaction_reference');
				$transaction_node->addChild('merchant_transaction_id', $order->attr('ecom_ref_no'));
				$transaction_node->addChild('gateway_transaction_id', $order->attr('transaction_id'));

				$order_xml = $sxml->asXml();
			}
		}

		if (!empty($order_xml)) {
			if (!file_put_contents($file, $order_xml)) {
				trigger_error('Could write the xml data to the file '.$file, E_USER_NOTICE);
			}
		}

		// We dont want the checkout action failure block the checkout process
		// So always return true

		return TRUE;

	}//end execute()


	/**
	 * Validates the checkout action.
	 *
	 * @access public
	 * @return boolean
	 */
	public static function isValid(Ecommerce_Checkout $checkout, $settings)
	{
		if (!isset($settings['save_xml'])) {
			$xml_settings = Array();
		} else {
			$xml_settings = $settings['save_xml'];
		}
		self::_fillDefaultValues($xml_settings);

		// Check if valid and return FALSE
		if (!isset($xml_settings['location']) || empty($xml_settings['location'])) {
			return FALSE;
		}//end if

		if (!is_dir($xml_settings['location']) || !is_writable($xml_settings['location'])) {
			return FALSE;
		}//end if

		return TRUE;

	}//end isValid()


	/**
	* Fill any missing values from the xml format with defaults
	*
	* @param array	&$xml_format	The xml settings
	*
	* @return void
	*/
	private static function _fillDefaultValues(&$xml_format) {
		if (!isset($xml_format['location'])) {
			$xml_format['location'] = '';
		}
		if (empty($xml_format['include_trans_ref'])) {
			$xml_format['include_trans_ref'] = '0';
		}//end if

	}//end _fillDefaultValues()


}//end class

?>
