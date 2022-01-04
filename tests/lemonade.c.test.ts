import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

function seconds_since_epoch(d: number) {
    return Math.floor(d / 1000);
}

describe("lemonade.c 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    const user = "test.p";
    let contractTester: any;
    let ledTokenTester: any;
    let ledTester: any;
    let errorCount = 0;
    const initialize = 10000;
    const stake = 10;

    describe("테스트가 시작되면, 블록체인이 연결 되어야 함", async () => {
        it("블록체인을 연결함", async () => {
            expect(await bc.checkConntection()).eq(true);
        });
    });

    if (isLocal) {
        describe("로컬블록체인이라면, 컨트랙트를 새로 배포해야 함", async () => {
            it("배포 계정을 필요한 경우 생성하고, 컨트랙트를 배포함", async () => {
                try {
                    contractTester = await bc.createAccountByCreator(
                        manager,
                        "led"
                    );
                    await contractTester.setContract("lemonade.c");
                    expect(contractTester.accountName).eq(manager);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    contractTester = await bc.getAccount(manager);
                    expect(contractTester.accountName).eq(manager);
                }
            });
            it("테스트를 위한 추가 계정들을 생성함", async () => {
                try {
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.createAccountByCreator(user, "led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.getAccount(user);
                }
            });
            it(`${user} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: user,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
        });
        it("led.code 권한 생성", async () => {
            try {
                const authData = {
                    threshold: 1,
                    keys: [
                        {
                            key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                            weight: 1
                        }
                    ],
                    accounts: [
                        {
                            permission:
                            {
                                actor: manager,
                                permission: "led.code"
                            },
                            weight: 1
                        }
                    ],
                    waits: [],
                }
                const actionResult = await ledTester.actions.updateauth(
                    {
                        account: manager,
                        permission: "active",
                        parent: "owner",
                        auth: authData,
                    },
                    [
                        {
                            actor: manager,
                            permission: "owner",
                        },
                    ]
                );
                expect(actionResult).to.have.all.keys([
                    "transaction_id",
                    "processed",
                ]);
            } catch (error) {
                console.log(
                    `ERROR ${errorCount}: cannot transfer: ${error}`
                );
                errorCount += 1;
            }
        });

        describe("Product: addproduct()", async () => {
            it(`${manager} 유저가 일반 예금 상품을 생성함`, async () => {
                try {
                    const actionResult = await contractTester.actions.addproduct(
                        {
                            product_name: "normal",
                            minimum_yield: 1.18, // 18% per year
                            maximum_yield: 1.18, 
                            amount_per_account: "0.0000 LED", 
                            maximum_amount_limit: null,
                            duration: null
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot add product: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.products();
                if (debug) console.log(`Products\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ name: "normal" });
            });
            
            it(`${manager} 유저가 더미 상품을 생성함`, async () => {
                try {
                    const actionResult = await contractTester.actions.addproduct(
                        {
                            product_name: "dummy",
                            minimum_yield: 1.99,
                            maximum_yield: 1.99, 
                            amount_per_account: "1.0000 LED", 
                            maximum_amount_limit: null,
                            duration: null
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot add product: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.products();
                if (debug) console.log(`Products\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
                expect(onlyCreated).to.deep.include({ name: "dummy" });
            });
        });

        describe("Product: rmproduct()", async () => {
            it(`${manager} 유저가 더미 상품을 제거함`, async () => {
                try {
                    const actionResult = await contractTester.actions.rmproduct(
                        {
                            product_name: "dummy",
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot remove product: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 제거 확인`, async () => {
                const tableResult = await contractTester.tables.products();
                if (debug) console.log(`Products\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
                expect(onlyCreated).to.equal(undefined);
            });
        });

        describe("Staking: stake()", async () => {
            it(`${user} 유저가 staking을 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `normal`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`Accounts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ balance: `${stake}.0000 LED` });
            });
            it(`${user} 계정 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('led.token', user, 'LED');
                console.log(balance);
                expect(balance[0]).to.equal(`${initialize-stake}.0000 LED`);
            });
        });
    }
});
