<?php
/**
 * DALInsertParser.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author     Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

require_once 'DAL/Parsers/DALQueryParser.inc';
require_once 'DAL/DALBaker.inc';
require_once 'DAL/Parsers/DALWhereParser.inc';

/**
 * DALTruncateParser.
 *
 * Truncate query parser. This is an alias for "Delete All", as the normal
 * Delete query provides no ability to unconditionally delete all of a
 * table's entries (as the table name is placed in the WHERE portion).
 *
 * @since 4.0.0
 */
class DALTruncateParser extends DALQueryParser
{


    /**
     * Constructor.
     *
     * Private to avoid instantiating the object.
     * All DALBaker methods should be called statically.
     *
     * @since 4.0.0
     */
    private function __construct()
    {

    }//end __construct()


    /**
     * Construct the TRUNCATE query from XML element.
     *
     * array(
     *  'TRUNCATE' => array(
     *               'from'  => string,
     *              ),
     * )
     *
     * @param DomElement $xmlQuery The query element.
     *
     * @since  4.0.0
     * @return array
     */
    public static function parse(DomElement $xmlQuery)
    {
        $query     = array();
        $deleteTag = $xmlQuery->getElementsByTagName('truncate')->item(0);

        if ($deleteTag !== NULL) {
            $query['TRUNCATE'] = array();

            // There must be a where tag and should have been checked before.
            $tableTag = $deleteTag->getElementsByTagName('table')->item(0);
            $query['TRUNCATE']['from'] = $tableTag->nodeValue;
        }//end if

        return $query;

    }//end parse()


    /**
     * Validates DELETE query.
     *
     * Throws DALParserException. Note: This validation checks required XML
     * elements and attributes. It does not check if tables, columns etc
     * exists in the system.
     *
     * @param DomElement $query The query element.
     *
     * @since  4.0.0
     * @return string
     * @throws DALParserException If the DELETE query is malformed.
     */
    public static function validate(DomElement $query)
    {
        $deleteTag = $query->getElementsByTagName('truncate')->item(0);
        if ($deleteTag === NULL) {
            throw new DALParserException('Truncate tag not found.');
        }

        // Must have where tag.
        $tableTag = $deleteTag->getElementsByTagName('table')->item(0);
        if ($tableTag === NULL) {
            $msg = 'Truncate queries must have a table tag, specifying the table to be cleared.';
            throw new DALParserException($msg);
        } else if (trim($tableTag->nodeValue) === '') {
            $msg = 'Truncate queries must have a table tag, specifying the table to be cleared.';
            throw new DALParserException($msg);
        }

    }//end validate()


}//end class

?>