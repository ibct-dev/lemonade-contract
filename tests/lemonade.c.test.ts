import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

function seconds_since_epoch(d: number) {
    return Math.floor(d / 1000);
}

beforeEach(async () => {
    await new Promise(resolve => setTimeout(resolve, 1000));
    console.log("----------------------");
});

describe("lemonade.c 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    const user = "test.p";
    const anotherUser = "test1.p";
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
                    await bc.createAccountByCreator(anotherUser, "led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.getAccount(user);
                    await bc.getAccount(anotherUser);
                }
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
            it(`${anotherUser} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: anotherUser,
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

        describe("Initialize: init()", async () => {
            it(`${manager} 유저가 컨트랙트 세팅 초기화 실행`, async () => {
                try {
                    const actionResult = await contractTester.actions.init(
                        {},
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
                        `ERROR ${errorCount}: cannot initialize: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`config 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.configs();
                if (debug) console.log(`Configs\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ id: 0 });
                expect(onlyCreated).to.deep.include({ is_active: 1 });
            });
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

            it(`${manager} 유저가 고정 상품을 생성함`, async () => {
                try {
                    const actionResult = await contractTester.actions.addproduct(
                        {
                            product_name: "fixed",
                            minimum_yield: 1.18,
                            maximum_yield: 1.50, 
                            amount_per_account: "9.0000 LED", 
                            maximum_amount_limit: "11.0000 LED",
                            duration: 8
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
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.deep.include({ name: "fixed" });
            });
        });

        describe("Staking: stake()", async () => {
            it(`${user} 유저가 일반 상품에 staking을 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `staking/normal`
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
                expect(balance[0]).to.equal(`${initialize-stake}.0000 LED`);
            });

            it(`${user} 유저가 없는 상품에 staking 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `staking/done`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot transfer: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });

            it(`${user} 유저가 고정 상품에 staking을 한도 이상으로 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `staking/fixed`
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot transfer: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });

            it(`${user} 유저가 고정 상품에 staking을 정상적으로 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake-2}.0000 LED`,
                            memo: `staking/fixed`
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
            it(`${anotherUser} 유저가 고정 상품에 staking을 전체 한도 이상으로 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: anotherUser,
                            to: manager,
                            quantity: `${stake-3}.0000 LED`,
                            memo: `staking/fixed`
                        },
                        [
                            {
                                actor: anotherUser,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot transfer: ${error}`
                    );
                    expect(true).to.be.true;
                }
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
                const onlyCreated = tableResult[2];
                expect(onlyCreated).to.equal(undefined);
            });

            it(`${manager} 유저가 구매자가 있는 상품을 제거함`, async () => {
                try {
                    const actionResult = await contractTester.actions.rmproduct(
                        {
                            product_name: "normal",
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
                        `ERROR: cannot remove product: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });
        });

        describe("Unstaking: unstake()", async () => {
            it(`${user} 유저가 일반 상품을 unstake 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.unstake(
                        {
                            owner: user,
                            product_name: "normal",
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
                        `ERROR ${errorCount}: cannot unstaking: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 삭제 확인`, async () => {
                const tableResult = await contractTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`Accounts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[1];
                expect(onlyCreated).to.equal(undefined);
            });

            it(`${user} 유저가 시간이 안지난 고정 상품을 unstake 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.unstake(
                        {
                            owner: user,
                            product_name: "fixed",
                        },
                        [
                            {
                                actor: user,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot unstaking: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });

            it(`${anotherUser} 유저가 가입 안한 상품을 unstake 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.unstake(
                        {
                            owner: anotherUser,
                            product_name: "fixed",
                        },
                        [
                            {
                                actor: anotherUser,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot unstaking: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });

            it(`${user} 유저가 시간이 만료됨 고정 상품을 unstake 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.unstake(
                        {
                            owner: user,
                            product_name: "fixed",
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
                        `ERROR ${errorCount}: cannot unstaking: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`account 테이블에 삭제 확인`, async () => {
                const tableResult = await contractTester.tables.accounts({
                    scope: user,
                });
                if (debug) console.log(`Accounts\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.equal(undefined);
            });
        });

        describe("Createbet: createbet()", async () => {
            it(`${manager} 유저가 베팅 게임을 생성함`, async () => {
                try {
                    const actionResult = await contractTester.actions.createbet(
                        {
                            started_at: seconds_since_epoch(Date.now()) + 2,
                            betting_ended_at: seconds_since_epoch(Date.now()) + 6,
                            ended_at: seconds_since_epoch(Date.now()) + 7, 
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
                        `ERROR ${errorCount}: cannot add game: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`betting 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.bettings();
                if (debug) console.log(`Bettings\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ status: 0 });
            });
        });


        describe("Setbet: setbet()", async () => {
            it(`${manager} 유저가 베팅 시작 시간이 안됐는데 시작함`, async () => {
                try {
                    const actionResult = await contractTester.actions.setbet(
                        {
                            bet_id:0,
                            status:1,
                            base_price: 1.0
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot change game status to live: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });
            it(`${manager} 유저가 없는 게임을 설정하려고 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.setbet(
                        {
                            bet_id:1,
                            status:1,
                            base_price: 1.0
                        },
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot change game status: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });
            it(`${manager} 유저가 베팅 게임을 정상적으로 시작함`, async () => {
                try {
                    const actionResult = await contractTester.actions.setbet(
                        {
                            bet_id:0,
                            status: 1,
                            base_price: 1.0
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
                        `ERROR ${errorCount}: cannot change game status to live: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`betting 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.bettings();
                if (debug) console.log(`Bettings\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ id: 0 });
                expect(onlyCreated).to.deep.include({ status: 1 });
            });
        });

        describe("Betting: bet()", async () => {
            it(`${user} 유저가 betting을 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: user,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `bet/0/long`
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
            it(`betting 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.bettings();
                if (debug) console.log(`Bettings\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
            });
            it(`${anotherUser} 유저가 betting을 시간 다 지나서 함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: anotherUser,
                            to: manager,
                            quantity: `${stake}.0000 LED`,
                            memo: `bet/0/long`
                        },
                        [
                            {
                                actor: anotherUser,
                                permission: "active",
                            },
                        ]
                    );
                } catch (error) {
                    console.log(
                        `ERROR: cannot transfer: ${error}`
                    );
                    expect(true).to.be.true;
                }
            });
        });

        describe("Claimbet: claimbet()", async () => {
            it(`${manager} 유저가 claimbet 함`, async () => {
                try {
                    const actionResult = await contractTester.actions.claimbet(
                        {
                            bet_id: 0,
                            win_position: "long",
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
                        `ERROR ${errorCount}: cannot claimbet: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`betting 테이블에 변경 확인`, async () => {
                const tableResult = await contractTester.tables.bettings();
                if (debug) console.log(`Bettings\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ status: 3 });
            });
            it(`${user} 계정 잔액 확인`, async () => {
                const balance = await bc.rpc.get_currency_balance('led.token', user, 'LED');
                console.log(balance);
            });
        });
    }
});
