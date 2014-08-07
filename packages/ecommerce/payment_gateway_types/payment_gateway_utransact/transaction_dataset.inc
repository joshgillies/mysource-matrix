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
* $Id: transaction_dataset.inc,v 1.3 2012/08/30 00:58:34 ewang Exp $
*
*/


/**
* Transaction_Dataset
*
* This class represents a new transaction dataset
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Transaction_Dataset
{
	
	/**
	 * Transaction table, contains 1 mandatory row 
	 *
	 * @var array
	 */
	private $_transaction;
	
	/**
	 * Purchaser table, contains 0..1 rows
	 *
	 * @var array
	 */
	private $_purchaser;
	

	/**
	 * Line Items table, contains 0..* rows 
	 *
	 * @var array
	 */
	private $_line_items;


	/**
	 * Line Items counter 
	 *
	 * @var int
	 */
	private $_line_item_count;

	/**
	 * Constructor
	 *
	 */
	function __construct()
	{   
        $this->_line_item_count = 0;
		
	}//end constructor
	
	
	/**
	* Return the transaction dataset array
	*
	* @return array
	* @access public
	*/	
    function getTransactionDatasetArray()
    {

        $line_items = Array();

        foreach($this->_line_items as $id => $line_item) {
            $line_items["LineItem id=\"".$id."\""] = $line_item;
        }

        $dataset = Array(
                   "NewTransactionDataset"     => Array(
                                       "Purchaser"     => $this->_purchaser,
                                       "Transaction"   => $this->_transaction,
                                       "0"                 => $line_items,
                                   ),
                           );
                           
        return $dataset;
    }	

	/**
	* Return the transaction dataset in xml format
	*
	* @return object
	* @access public
	*/	
	function getTransactionDatasetXML()
	{
        // Get the new transaction XSD content
		$new_transaction_xsd = $this->_getNewTransactionXSD();

		// Get the new transaction xml namespace from the xsd content and add it to the new transaction dataset
		preg_match('|xmlns="(.*?)"|msi', $new_transaction_xsd, $match);
		$xml_ns =  (isset($match[1])) ? $match[1] : '';
		
		// Get the new transaction dataset in XML format
		$new_transaction_dataset = $this->_convertToXML($this->getTransactionDatasetArray());
		
    
		if ($xml_ns) {
			$new_transaction_dataset = str_replace("<NewTransactionDataset>", "<NewTransactionDataset xmlns=\"$xml_ns\">", $new_transaction_dataset);
		}
        
        return $new_transaction_xsd.$new_transaction_dataset;
	}
	
	
	/**
	* Converts an array structure into an xml string
	*
	* Example: 
	*		Array(
	*			"Profile" => Array(
	*				"Name" 	=> "Maxx",
	*				"Age"	=> "22",
	*				),
	*			)
	* will be converted into string:
	*	"
	*	<Profile>
	*		<Name>Maxx</Name>
	*		<Age>22</Age>
	*	</Profile>
	*	"
	*
	* @parm Array $str
	*
	* @return string
	* @access public
	*/	
	function _convertToXML($str)
	{
        if (!is_array($str))
            return $str;
        
        $xml = '';
        foreach($str as $key => $value) {
            if (is_int($key))        
				$xml .= $this->_convertToXML($value);
			else
				  $xml .= "\n<$key>".$this->_convertToXML($value)."</".preg_replace('|\s.*$|', '', $key).">";
        }
        
        return $xml;
	}


	/**
	* Set purchaser details
	*
	* @param string	$fname	    First name
	* @param string	$sname	    Surname
	* @param string	$email	    Email
	* @param string	$address1	Address line 1
	* @param string	$address1	Address line 2
    * @param string	$suburb	    Suburb
    * @param string	$city       City
    * @param int	$postcode   Post code
	*
	* @return boolean
	* @access public
	*/	
	function addPurchaserRow($fname, $sname, $email, $address1, $address2, $suburb, $city, $postcode)
	{   
        $this->_purchaser = Array(
                            0 => Array(
                            "Firstname"         => $fname,
                            "Surname"           => $sname,
                            "AddressLine1"      => $address1,
                            "City"              => $city,
                            ),
                    );
                    
        // These fields are not mandatory
        if (!empty($email)) {
            $this->_purchaser[0]["Email"] = $email;
        }
        if (!empty($address2)) {
            $this->_purchaser[0]["AddressLine2"] = $address2;
        }
        if (!empty($suburb)) {
            $this->_purchaser[0]["Suburb"] = $suburb;
        }
        if (!empty($postcode)) {
            $this->_purchaser[0]["Postcode"] = $postcode;
        }
                    
        return TRUE;	
	}
	

	/**
	* Set transaction details
	*
	* @param string	$reference_id
	* @param string	$credit_card
	* @param string	$expiry_date
	* @param string	$card_holder_name
	* @param int	$merchant_id
    * @param string	$payment_model
	* @param string $gst_rate
	*
	* @return boolean
	* @access public
	*/	
	function addTransactionRow($reference_id, $credit_card, $expiry_date, $card_holder_name, $merchant_id='', $payment_model='', $gst_rate=0)
	{   
        $this->_transaction = Array(
                            0 => Array(
                            "ReferenceId"       => $reference_id,
                            "CreditCard"        => $credit_card,
                            "ExpiryDate"        => $expiry_date,
                            "CardHolderName"    => $card_holder_name,
							"GstRate"			=> $gst_rate,
                            ),
                        );
       

        // MerchantId and PaymentModel are not mandatory fields
        if (!empty($merchant_id)) {
            $this->_transaction[0]['MerchantId']  = $merchant_id;
        }        
        if (!empty($payment_model)) {
            $this->_transaction[0]['PaymentModel']  = $payment_model;
        }        
                        
        return TRUE;
	}
	

	/** 
	* Create an line item and add it to the list
	*
	* @param string	$application_username
	* @param string	$name
	* @param string	$description
	* @param int	$quantity
    * @param string	$refund_type
    * @param string	$refund_expiry_date
    * @param float	$amount
    * @param string	$product_code
    * @param string	$financial_code
    * @param string	$financial_values
    * @param string	$additional_info
	*
	* @return boolean
	* @access public
	*/	
	function addLineItemRow($application_username, $name, $description, $quantity, $refund_type, $refund_expiry_date, $amount, $product_code, $financial_code, $financial_values, $additional_info)
	{   
        // Set the refernece id based on the transaction reference id
        $reference_id = $this->_transaction[0]['ReferenceId'].'/'.$this->_line_item_count++;
        
        $line_item_dataset = Array(
                "ReferenceId"           => $reference_id,
                "Name"                  => $name,
                "Description"           => $description,
                "Quantity"              => $quantity,
                "RefundType"            => $refund_type,
                "Amount"                => $amount,
            );
                
        // These fields are not mandatory
        if (!empty($application_username)) {
            $line_item_dataset["ApplicationUsername"] = $application_username;
        }
        if (!empty($refund_expiry_date)) {
            $line_item_dataset["RefundExpiryDate"] = $refund_expiry_date;
        }        
        if (!empty($product_code)) {
            $line_item_dataset["ProductCode"] = $product_code;
        }
        if (!empty($financial_code)) {
            $line_item_dataset["FinancialCode"] = $financial_code;
        }
        if (!empty($financial_values)) {
            $line_item_dataset["FinancialValues"] = $financial_values;
        }
        if (!empty($additional_info)) {
            $line_item_dataset["AdditionalInfo"] = $additional_info;
        }
        
        $this->_line_items[$reference_id] = $line_item_dataset;
        
        return TRUE;

	}
	

	/**
	* Returns XSD for the new transaction dataset
	*
	* @return object
	* @access public
	*/	
	function _getNewTransactionXSD()
	{
        $new_transaction_xsd = <<<XSD_CONTENT
        
<xs:schema id="Transaction" targetNamespace="http://www.paymentgateway.co.nz/_schemas/Transaction.xsd"
elementFormDefault="qualified" attributeFormDefault="qualified"
xmlns="http://www.paymentgateway.co.nz/_schemas/Transaction.xsd"
xmlns:mstns="http://www.paymentgateway.co.nz/_schemas/Transaction.xsd"
xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:msdata="urn:schemas-microsoft-com:xml-msdata">
<xs:element name="NewTransactionDataset" msdata:IsDataSet="true">
<xs:complexType>
<xs:choice maxOccurs="unbounded">
<xs:element name="Purchaser">
<xs:complexType>
<xs:sequence>
<xs:element name="Firstname" type="xs:string" />
<xs:element name="Surname" type="xs:string" />
<xs:element name="Email" type="xs:string" minOccurs="0" />
<xs:element name="AddressLine1" type="xs:string" />
<xs:element name="AddressLine2" type="xs:string" minOccurs="0" />
<xs:element name="Suburb" type="xs:string" minOccurs="0" />
<xs:element name="City" type="xs:string" />
<xs:element name="PostCode" type="xs:int" minOccurs="0" />
</xs:sequence>
</xs:complexType>
</xs:element>
<xs:element name="Transaction">
<xs:complexType>
<xs:sequence>
<xs:element name="ReferenceId" type="xs:string" />
<xs:element name="CreditCard" type="xs:string" />
<xs:element name="ExpiryDate" type="xs:string" />
<xs:element name="CardHolderName" type="xs:string" />
<xs:element name="MerchantId" type="xs:int" minOccurs="0" />
<xs:element name="PaymentModel" type="xs:string" minOccurs="0" />
<xs:element name="GstRate" type="xs:decimal" minOccurs="0" />
</xs:sequence>
</xs:complexType>
</xs:element>
<xs:element name="LineItem">
<xs:complexType>
<xs:sequence>
<xs:element name="ReferenceId" type="xs:string" />
<xs:element name="ApplicationUsername" type="xs:string" minOccurs="0" />
<xs:element name="Name" type="xs:string" />
<xs:element name="Description" type="xs:string" />
<xs:element name="Quantity" type="xs:int" />
<xs:element name="RefundType" type="xs:string" />
<xs:element name="RefundExpiryDate" type="xs:dateTime" minOccurs="0" />
<xs:element name="Amount" type="xs:decimal" />
<xs:element name="ProductCode" type="xs:string" minOccurs="0" />
<xs:element name="FinancialCode" type="xs:string" minOccurs="0" />
<xs:element name="FinancialValues" type="xs:string" minOccurs="0" />
<xs:element name="AdditionalInfo" type="xs:string" minOccurs="0" />
</xs:sequence>
</xs:complexType>
</xs:element>
</xs:choice>
</xs:complexType>
<xs:key name="NewTransactionDatasetKey1" msdata:PrimaryKey="true">
<xs:selector xpath=".//mstns:LineItem" />
<xs:field xpath="mstns:ReferenceId" />
</xs:key>
<xs:key name="NewTransactionDatasetKey2" msdata:PrimaryKey="true">
<xs:selector xpath=".//mstns:Transaction" />
<xs:field xpath="mstns:ReferenceId" />
</xs:key>
</xs:element>
</xs:schema>
        
XSD_CONTENT;

        return $new_transaction_xsd;

	}//end _getNewTransactionXSD()
	
	
}//end class

?>
