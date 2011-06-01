<?php
/**
 * DALWhereParser.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author     Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

require_once 'DAL/Parsers/DALQueryParser.inc';
require_once 'DAL/Parsers/DALSelectParser.inc';
require_once 'DAL/DALBaker.inc';
require_once 'XML/XML.inc';

/**
 * DALWhereParser.
 *
 * Where clause parser. Parses a where clause.
 *
 * @since 4.0.0
 */
class DALWhereParser extends DALQueryParser
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
     * Returns the array form of the SELECT query.
     *
     * @param DomElement $parent Query element.
     * @param string     $type   Type of the clause, where or having.
     *
     * @since  4.0.0
     * @return array
     */
    public static function parse(DomElement $parent, $type='where')
    {
        $where    = array();
        $whereTag = NULL;

        foreach ($parent->childNodes as $tag) {
            if ($tag->nodeType !== XML_ELEMENT_NODE) {
                continue;
            } else if ($tag->tagName !== $type) {
                continue;
            } else {
                $whereTag = $tag;
            }
        }

        if ($whereTag !== NULL) {
            $where[strtoupper($type)] = self::_getWhereConditions($whereTag);
        }

        $joins = self::_praseJoinsForSelect($parent);
        if (empty($joins) === FALSE) {
            $where = array_merge($where, $joins);
        }

        return $where;

    }//end parse()


    /**
     * Constructs the SQL's conditions from XML node.
     *
     * @param DomElement $parent Select Node of the XML.
     * @param string     $type   Type of the where condition.
     * @param integer    $level  Indentation level.
     *
     * @since  4.0.0
     * @return array
     */
    private static function _getWhereConditions(DomElement $parent, $type='AND', $level=1)
    {
        $conditions = array();
        foreach ($parent->childNodes as $cond) {
            if ($cond->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }

            if (DALBaker::getComparisonOperators($cond->tagName) !== '') {
                $currentCond            = array();
                $currentCond['compare'] = array();
                $currentCond['type']    = $cond->tagName;

                $offset = 0;
                if ($cond->getAttribute('table') !== '') {
                    // Field compare.
                    $currentCond['compare']['table']  = $cond->getAttribute('table');
                    $currentCond['compare']['column'] = $cond->getAttribute('column');
                } else if (XML::hasChildElements($cond) === TRUE) {
                    $offset = 1;
                    $value  = self::parseSingleField(XML::getFirstChildElement($cond));

                    $currentCond['compare'] = $value;
                }

                $value = '';
                if (XML::hasChildElements($cond) === TRUE) {
                    $child = XML::getChildAt($cond, $offset);
                    if ($child !== NULL) {
                        $value = self::parseSingleField($child);
                    }
                } else {
                    $value = $cond->nodeValue;
                }

                $currentCond['to']   = $value;
                $conditions[$type][] = $currentCond;
            } else if (($cond->tagName === 'in') || ($cond->tagName === 'not-in')) {
                // This is an IN/NOT-IN condition.
                $conditions[$type][] = self::parseInClause($cond, $level);
            } else if (($cond->tagName === 'exists') || ($cond->tagName === 'not-exists')) {
                $conditions[$type][] = self::parseExistsClause($cond, $level);
            } else if ($cond->tagName === 'or') {
                $conditions[$type][] = self::_getWhereConditions($cond, 'OR');
            } else if ($cond->tagName === 'and') {
                $conditions[$type][] = self::_getWhereConditions($cond, 'AND');
            } else if ($cond->tagName === 'hook') {
                $conditions[$type][] = DALBaker::getHookPrefix().$cond->getAttribute('id');
            }//end if

        }//end foreach

        return $conditions;

    }//end _getWhereConditions()


    /**
     * Constructs the SQL's joins from XML node.
     *
     * Returns the part of where clause.
     * array(
     *  'JOIN' => array(
     *             0 => array(
     *                   'ARGS' => array( // must have 2 elements.
     *                              0 => array(
     *                                    'table'  => string,
     *                                    'column' => string,
     *                                   ),
     *                              1 => array(
     *                                    'table'  => string,
     *                                    'column' => string,
     *                                   ),
     *                             ),
     *                   'TYPE' => string,
     *                  ),
     *            ),
     * )
     *
     * @param DomElement $select Select Node of the XML.
     *
     * @since  4.0.0
     * @return array
     */
    private static function _praseJoinsForSelect(DomElement $select)
    {
        $joinList = array();
        $joinsTag = $select->getElementsByTagName('joins')->item(0);
        if ($joinsTag !== NULL) {
            $joinList['JOIN'] = array();

            $joins = $joinsTag->getElementsByTagName('join');
            foreach ($joins as $join) {
                $condition          = array();
                $condition['ARGS']  = self::parseChildFields($join);
                $condition['type']  = 'equal';
                $joinList['JOIN'][] = $condition;
            }
        }

        return $joinList;

    }//end _praseJoinsForSelect()


    /**
     * Validates where clause.
     *
     * Throws DALParserException.
     *
     * @param DomElement $parent The query element.
     *
     * @since  4.0.0
     * @return string
     * @throws DALParserException If validation fails.
     */
    public static function validate(DomElement $parent)
    {
        $condSigns = DALBaker::getComparisonOperators();

        // Add in and not-in to conditions.
        $condSigns['in']     = '';
        $condSigns['not-in'] = '';

        // Add exists and not-exists to conditions.
        $condSigns['exists']     = '';
        $condSigns['not-exists'] = '';

        // Loop through each childNode and check their contents.
        foreach ($parent->childNodes as $cond) {
            if ($cond->nodeType !== XML_ELEMENT_NODE) {
                continue;
            }

            $tagName = $cond->tagName;

            if (isset($condSigns[$tagName]) === TRUE) {
                if ($tagName === 'is-null' || $tagName === 'not-null') {
                    if (XML::hasChildElements($cond) === FALSE) {
                        $msg = $tagName.' tag must have child.';
                        throw new DALParserException($msg);
                    }

                    continue;
                }

                if ((Xml::childCount($cond) !== 2) && ($tagName !== 'exists') && ($tagName !== 'not-exists')) {
                    if ($cond->getAttribute('table') === '') {
                        $msg = 'WHERE condition with no table attribute.';
                        throw new DALParserException($msg);
                    }

                    if ($cond->getAttribute('column') === '') {
                        $msg = 'WHERE condition with no column attribute.';
                        throw new DALParserException($msg);
                    }
                }

                if (($tagName === 'in') || ($tagName === 'not-in')) {
                    // We need additional checks for in and not-in.
                    $value  = $cond->getElementsByTagName('value')->item(0);
                    $select = $cond->getElementsByTagName('select')->item(0);
                    if (($value === NULL) && ($select === NULL)) {
                        if ($cond->nodeValue === '') {
                            $msg = $tagName.' must have value or select tag.';
                            throw new DALParserException($msg);
                        }
                    }

                    if ($select !== NULL) {
                        // Validate inner SEELECT query.
                        DALSelectParser::validate($cond);
                    }
                }

                if (($tagName === 'exists') || ($tagName === 'not-exists')) {
                    // We need additional checks for exists and not-exists.
                    $select = $cond->getElementsByTagName('select')->item(0);
                    if ($select === NULL) {
                        $msg = $tagName.' must have select tag.';
                        throw new DALParserException($msg);
                    }

                    DALSelectParser::validate($cond);
                }

            } else if (($tagName === 'or') || ($tagName === 'and')) {
                self::validate($cond);
            } else if ($tagName !== 'hook') {
                $msg = 'Found invalid WHERE comparison type "'.$tagName.'".';
                throw new DALParserException($msg);
            }//end if
        }//end foreach

    }//end validate()


}//end class

?>