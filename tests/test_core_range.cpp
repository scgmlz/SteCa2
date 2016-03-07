#include "test_core_range.h"
#include <core_types.h>

void TestCoreRange::testRange() {
  // Tests for Range
  // Basic Tests for default constructor
  {
    core::Range r;
    QVERIFY(!r.isValid());
  }

  // Basic Tests using one Argument constructor
  {
    qreal val1 = 5, val2 = 42;
    core::Range r(val1);
    QVERIFY(r.isValid());

    r = core::Range::infinite();
    QVERIFY(r.isValid());

    r.set(val2);
    QVERIFY(r.contains(val2));
    QVERIFY(!r.contains(val1));

    r.extend(val1);
    QCOMPARE(r.min, val1);
    QCOMPARE(r.max, val2);
  }

  // Basic Tests using two Argument constructor
  {
    qreal min = 2, max = 42;
    core::Range r(min,max);
    QVERIFY(r.isValid());

    for (int i = min; i<max; ++i) {
      QVERIFY(r.contains(core::Range(min,i)));
    }

    for (int i = min; i<max; ++i) {
      QVERIFY(r.contains(core::Range(i,max)));
    }

    r.safeSet(min,max);
    QCOMPARE(r.min,min);
    QCOMPARE(r.max,max);

    r.safeSet(max,min);
    QCOMPARE(r.min,min);
    QCOMPARE(r.max,max);

    QCOMPARE(r.safeFrom(max,min).min,core::Range(min,max).min);
    QCOMPARE(r.safeFrom(max,min).max,core::Range(min,max).max);

    r.extend(core::Range(min-max,max+min));
    QVERIFY(r.contains(core::Range(-40,44)));
    QVERIFY(r.intersects(core::Range(min,max+100)));
  }

  // Tests for Ranges
  // default Constructor
  {
    uint length = 3;
    core::Ranges rs;
    QVERIFY(rs.isEmpty());

    qreal min = 0, max = 3;
    for (uint i = 0; i<length; ++i) {
      rs.add(core::Range(min,max));
      min = max + 1;
      max = 2*max + 1;
    }

    min = 0; max= 3;
    QVERIFY(!rs.isEmpty());
    QCOMPARE(rs.count(),length);

    QCOMPARE(rs.at(0).min, 0.);
    QCOMPARE(rs.at(1).min, 4.);
    QCOMPARE(rs.at(2).min, 8.);

    QVERIFY(rs.rem(core::Range(min+1,max)));
    QCOMPARE(rs.at(0).min, 0.);
    QCOMPARE(rs.at(0).max, 1.);

    QVERIFY(!rs.add(core::Range(0., 1.)));
    QVERIFY(rs.add(core::Range(min,  max+1000.)));
    QCOMPARE(rs.at(0).min, 0.);
    QCOMPARE(rs.at(0).max, max+1000.);
  }

  //Tests for TI_Curve
  {
    {
      qreal tth = 1.2, inten = 3.14;
      core::TI_Curve t;

      // testing default Constructor, All data is NaN
      {
        QVERIFY(t.isEmpty());
        QVERIFY(t.getTth().isEmpty());
        QVERIFY(t.getInten().isEmpty());
        QVERIFY(qIsNaN(t.getTthRange().min));
        QVERIFY(qIsNaN(t.getTthRange().max));
        QVERIFY(qIsNaN(t.getIntenRange().min));
        QVERIFY(qIsNaN(t.getIntenRange().max));
      }

      //testing if input of data is correct
      {
        t.append(tth,inten);
        QCOMPARE(t.getTth().at(0),tth);
        QCOMPARE(t.getInten().at(0),inten);
        QCOMPARE(t.getTthRange().min,tth);
        QCOMPARE(t.getTthRange().max,tth);
        QCOMPARE(t.getIntenRange().min,inten);
        QCOMPARE(t.getIntenRange().max,inten);

        t.append(2*tth,0);
        QCOMPARE(t.getTth().at(1),      2*tth);
        QCOMPARE(t.getInten().at(1),    0.);
        QCOMPARE(t.getTthRange().min,   tth);
        QCOMPARE(t.getTthRange().max,   2*tth);
        QCOMPARE(t.getIntenRange().min, 0.);
        QCOMPARE(t.getIntenRange().max, inten);
        QVERIFY(t.isOrdered());

        t.clear();
        QVERIFY(t.isEmpty());
      }
    }
  }
}

// eof
